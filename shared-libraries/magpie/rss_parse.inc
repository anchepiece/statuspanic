<?php

/**
* Project:     MagpieRSS: a simple RSS integration tool
* File:        rss_parse.inc  - parse an RSS or Atom feed
*               return as a simple object.
*
* Handles RSS 0.9x, RSS 2.0, RSS 1.0, Atom 0.3, and Atom 1.0
*
* The lastest version of MagpieRSS can be obtained from:
* http://magpierss.sourceforge.net
*
* For questions, help, comments, discussion, etc., please join the
* Magpie mailing list:
* magpierss-general@lists.sourceforge.net
*
* @author           Kellan Elliott-McCrea <kellan@protest.net>
* @version          0.8
* @license          GPL
*
*/

define('RSS', 'RSS');
define('ATOM', 'Atom');

if (!defined('MAGPIE_DIR')) {
    define('MAGPIE_DIR', dirname(__FILE__) . DIRECTORY_SEPARATOR);
}

require_once (MAGPIE_DIR . 'rss_utils.inc');

/**
* Hybrid parser, and object, takes RSS as a string and returns a simple object.
*
* see: rss_fetch.inc for a simpler interface with integrated caching support
*
*/
class MagpieRSS {
    var $parser;
    
    var $current_item   = array();  // item currently being parsed
    var $items          = array();  // collection of parsed items
    var $channel        = array();  // hash of channel fields
    var $textinput      = array();
    var $image          = array();
    var $feed_type;
    var $feed_version;
    var $encoding       = '';       // output encoding of parsed rss
    
    var $_source_encoding = '';     // only set if we have to parse xml prolog
    
    var $ERROR = "";
    var $WARNING = "";
    
    # define some constants
    
    var $_ATOM_CONTENT_CONSTRUCTS = array(
        'content', 'summary', 'title', /* common */
        'info', 'tagline', 'copyright', /* Atom 0.3 */
        'rights', 'subtitle', /* Atom 1.0 */
    );
    
    var $_XHTML_CONTENT_CONSTRUCTS = array('body', 'div');
    var $_KNOWN_ENCODINGS    = array('UTF-8', 'US-ASCII', 'ISO-8859-1');

    # parser variables, useless if you're not a parser, treat as private
    
    var $stack              = array(); # parser stack
    var $inchannel          = false;
    var $initem             = false;
    
    var $incontent          = array(); # non-empty if in namespaced XML content field
    var $exclude_top        = false;   # true when Atom 1.0 type="xhtml"

    var $intextinput        = false;
    var $inimage            = false;
    var $current_namespace  = false;
    
    /**
     *  Set up XML parser, parse source, and return populated RSS object..
     *   
     *  @param string $source           string containing the RSS to be parsed
     *
     *  NOTE:  Probably a good idea to leave the encoding options alone unless
     *         you know what you're doing as PHP's character set support is
     *         a little weird.
     *
     *  NOTE:  A lot of this is unnecessary but harmless with PHP5 
     *
     *
     *  @param string $output_encoding  output the parsed RSS in this character 
     *                                  set defaults to ISO-8859-1 as this is PHP's
     *                                  default.
     *
     *                                  NOTE: might be changed to UTF-8 in future
     *                                  versions.
     *                               
     *  @param string $input_encoding   the character set of the incoming RSS source. 
     *                                  Leave blank and Magpie will try to figure it
     *                                  out.
     *                                  
     *                                   
     *  @param bool   $detect_encoding  if false Magpie won't attempt to detect
     *                                  source encoding. (caveat emptor)
     *
     */
    function MagpieRSS ($source, $output_encoding='ISO-8859-1', 
                        $input_encoding=null, $detect_encoding=true) 
    {   
        #
        # if PHP xml isn't compiled in, die
        #
        
        if (!function_exists('xml_parser_create')) {
            $this->error( "Failed to load PHP's XML Extension. " . 
                          "http://www.php.net/manual/en/ref.xml.php",
                           E_USER_ERROR );
        }
        
        list($parser, $source) = $this->create_parser($source, 
                $output_encoding, $input_encoding, $detect_encoding);
        
        
        if (!is_resource($parser)) {
            $this->error( "Failed to create an instance of PHP's XML parser. " .
                          "http://www.php.net/manual/en/ref.xml.php",
                          E_USER_ERROR );
        }

        $this->parser = $parser;
        
        #
        # pass in parser, and a reference to this object
        # setup handlers
        #
        
        xml_set_object( $this->parser, $this );
        xml_set_element_handler($this->parser, 
                'feed_start_element', 'feed_end_element' );
                        
        xml_set_character_data_handler( $this->parser, 'feed_cdata' ); 
    
        $status = xml_parse( $this->parser, $source );  # parse the feed
        
        if (! $status ) {
            $errorcode = xml_get_error_code( $this->parser );
            if ( $errorcode != XML_ERROR_NONE ) {
                $xml_error = xml_error_string( $errorcode );
                $error_line = xml_get_current_line_number($this->parser);
                $error_col = xml_get_current_column_number($this->parser);
                $errormsg = "$xml_error at line $error_line, column $error_col";

                $this->error( $errormsg );
            }
        }
        
        xml_parser_free( $this->parser );

        $this->normalize();
    }
    
    function feed_start_element($p, $element, &$attrs) {
        $el = $element = strtolower($element);
        $attrs = array_change_key_case($attrs, CASE_LOWER);
        
        # check for a namespace, and split if found
        # only if we're not inside a content tag
        
        if ( empty($this->incontent) ) {   
            $ns = false;
            
            if ( strpos( $element, ':' ) ) {
                list($ns, $el) = split( ':', $element, 2); 
            }
            
            if ( $ns and $ns != 'rdf' ) {
                $this->current_namespace = $ns;
            }
        }

        #  
        # if feed type isn't set, then this is first element of feed
        # identify feed from root element
        #
        
        if (!isset($this->feed_type) ) {
            if ( $el == 'rdf' ) {
                $this->feed_type = RSS;
                $this->feed_version = '1.0';
            }
            elseif ( $el == 'rss' ) {
                $this->feed_type = RSS;
                $this->feed_version = $attrs['version'];
            }
            elseif ( $el == 'feed' ) {
                $this->feed_type = ATOM;
                
                if ($attrs['xmlns'] == 'http://www.w3.org/2005/Atom') { // Atom 1.0
                    $this->feed_version = '1.0';
                }
                else { 
                    # Atom 0.3, probably.
                    $this->feed_version = $attrs['version'];
                }
                
                $this->inchannel = true;
            }
            return;
        }
    
        #
        # if we're inside a namespaced content construct, treat tags as text
        #
        
        if ( !empty($this->incontent) ) {
            if ((count($this->incontent) > 1) or !$this->exclude_top) {
                
                # if tags are inlined, then flatten
                
                $attrs_str = join(' ', 
                    array_map('map_attrs', 
                    array_keys($attrs), 
                    array_values($attrs))
                );
                    
                if (strlen($attrs_str)) { 
                    $attrs_str = ' '.$attrs_str; 
                }
        
                $this->append_content( "<{$element}{$attrs_str}>"  );
            }

            array_push($this->incontent, $el); # stack for parsing content XML
        } 
        
        elseif ( $el == 'channel' )  {
            $this->inchannel = true;
        }
    
        elseif ($el == 'item' or $el == 'entry' ) 
        {
            $this->initem = true;
            if ( isset($attrs['rdf:about']) ) {
                $this->current_item['about'] = $attrs['rdf:about']; 
            }
        }
        elseif ( 
            $this->feed_type == RSS and 
            $this->current_namespace == '' and 
            $el == 'textinput' ) 
        {
            # else we're in the default namespace of an RSS feed,
            # record textinput or image fields
            
            $this->intextinput = true;
        }
        elseif (
            $this->feed_type == RSS and 
            $this->current_namespace == '' and 
            $el == 'image' ) 
        {
            $this->inimage = true;
        }
        else {
            // set stack[0] to current element
            
            # Atom support many links per containing element.
            # Magpie treats link elements of type rel='alternate'
            # as being equivalent to RSS's simple link element.

            $atom_link = false;
            
            if ($this->feed_type == ATOM and $el == 'link') {
                $atom_link = true;
                if (isset($attrs['rel']) and $attrs['rel'] != 'alternate') {
                    $el = $el . "_" . $attrs['rel'];  // pseudo-element names for Atom link elements
                }
            }
            elseif ( $this->feed_type == ATOM and 
                in_array($el, $this->_ATOM_CONTENT_CONSTRUCTS) ) 
            {
                # handle atom content constructs
                
                # avoid clashing w/ RSS mod_content
                if ($el == 'content' ) {
                    $el = 'atom_content';
                }

                #
                # assume that everything accepts namespaced XML
                # (that will pass through some non-validating feeds;
                # but so what? this isn't a validating parser)
                #
                
                $this->incontent = array();
                array_push($this->incontent, $el); // start a stack

                if ( isset($attrs['type']) and trim(strtolower($attrs['type']))=='xhtml') {
                    $this->exclude_top = true;
                } else {
                    $this->exclude_top = false;
                }
            }
            elseif (($this->current_namespace=='xhtml' or 
                    (isset($attrs['xmlns']) and $attrs['xmlns'] == 'http://www.w3.org/1999/xhtml'))
                    and in_array($el, $this->_XHTML_CONTENT_CONSTRUCTS) )
            {
                # Handle inline XHTML body elements --CWJ
                  
                $this->current_namespace = 'xhtml';
                $this->incontent = array();
                array_push($this->incontent, $el); // start a stack
                $this->exclude_top = false;
            }

            array_unshift($this->stack, $el);
            $elpath = join('_', array_reverse($this->stack));

            $n = $this->element_count($elpath);
            $this->element_count($elpath, $n+1);

            if ($n > 0) {
                array_shift($this->stack);
                array_unshift($this->stack, $el.'#'.($n+1));  # TODO: revisit this
                $elpath = join('_', array_reverse($this->stack));
            }

            # this makes the baby Jesus cry, but we can't do it in normalize()
            # because we've made the element name for Atom links unpredictable
            # by tacking on the relation to the end. -CWJ
            
            if ($atom_link and isset($attrs['href'])) {
                $this->append($elpath, $attrs['href']);
            }
        
            # add attributes
            if (count($attrs) > 0) {
                $this->append($elpath.'@', join(',', array_keys($attrs)));
                foreach ($attrs as $attr => $value) {
                    $this->append($elpath.'@'.$attr, $value);
                }
            }
        }
    }



    function feed_cdata ($p, $text) {
        
        if ($this->incontent) {
            $this->append_content( $text );
        }
        else {
            $current_el = join('_', array_reverse($this->stack));
            $this->append($current_el, $text);
        }
    }
    
    function feed_end_element ($p, $el) {
        $el = strtolower($el);

        if ( $this->incontent ) {
            $opener = array_pop($this->incontent);

            # Don't get bamboozled by namespace voodoo
            if (strpos($el, ':')) { 
                list($ns, $closer) = split(':', $el); 
            }
            else { 
                $ns = false; $closer = $el; 
            }

            # Don't get bamboozled by our munging of <atom:content>, either
            if ($this->feed_type == ATOM and $closer == 'content') {
                $closer = 'atom_content';
            }

            # balance tags properly
            # NOTE: i don't think this is actually neccessary

            if ($opener != $closer) {
                array_push($this->incontent, $opener);
                $this->append_content("<$el />");
            }
            elseif ($this->incontent) { 
                # in the content construct
                
                if ((count($this->incontent) > 1) or !$this->exclude_top) {
                    $this->append_content("</$el>");
                }
            }
            else { 
                # shift the opening of the content construct off the normal stack
                array_shift( $this->stack );
            }
        }
        elseif ( $el == 'item' or $el == 'entry' ) {
            $this->items[] = $this->current_item;
            $this->current_item = array();
            $this->initem = false;

            $this->current_category = 0;
        }
        elseif ($this->feed_type == RSS and 
                $this->current_namespace == '' and $el == 'textinput' ) {
            $this->intextinput = false;
        }
        elseif ($this->feed_type == RSS and 
                $this->current_namespace == '' and $el == 'image' ) {
            $this->inimage = false;
        }
        elseif ($el == 'channel' or $el == 'feed' ) {
            $this->inchannel = false;
        }
        else {
            array_shift( $this->stack );
        }
        
        if ( !$this->incontent ) { 
            # Don't munge the namespace after finishing with elements in namespaced content constructs -CWJ
            $this->current_namespace = false;
        }
    }
    
    function concat (&$str1, $str2="") {
        if (!isset($str1) ) {
            $str1="";
        }
        $str1 .= $str2;
    }
    
    function append_content($text) {
        if ( $this->initem ) {
            if ($this->current_namespace) {
                $this->concat( $this->current_item[$this->current_namespace][ reset($this->incontent) ], $text );
            } else {
                $this->concat( $this->current_item[ reset($this->incontent) ], $text );
            }
        }
        elseif ( $this->inchannel ) {
            if ($this->current_namespace) {
                $this->concat( $this->channel[$this->current_namespace][ reset($this->incontent) ], $text );
            }
            else {
                $this->concat( $this->channel[ reset($this->incontent) ], $text );
            }
        }
    }
    
    # smart append - field and namespace aware
    function append($el, $text) {
        if (!$el) {
            return;
        }
        
        if ( $this->current_namespace ) {
            if ( $this->initem ) {
                $this->concat(
                    $this->current_item[ $this->current_namespace ][ $el ], $text );
            }
            elseif ($this->inchannel) {
                $this->concat(
                    $this->channel[ $this->current_namespace][ $el ], $text );
            }
            elseif ($this->intextinput) {
                $this->concat(
                    $this->textinput[ $this->current_namespace][ $el ], $text );
            }
            elseif ($this->inimage) {
                $this->concat(
                    $this->image[ $this->current_namespace ][ $el ], $text );
            }
        }
        else {
            if ( $this->initem ) {
                $this->concat( $this->current_item[ $el ], $text );
            }
            elseif ($this->intextinput) {
                $this->concat( $this->textinput[ $el ], $text );
            }
            elseif ($this->inimage) {
                $this->concat( $this->image[ $el ], $text );
            }
            elseif ($this->inchannel) {
                $this->concat( $this->channel[ $el ], $text );
            }
            
        }
    }

    # smart count - field and namespace aware
    function element_count ($el, $set = NULL) {
        if (!$el) {
            return;
        }
        
        if ( $this->current_namespace ) {
            if ( $this->initem ) {
                if (!is_null($set)) { 
                    $this->current_item[ $this->current_namespace ][ $el.'#' ] = $set; 
                }
                
                $ret = (isset($this->current_item[ $this->current_namespace ][ $el.'#' ]) ?
                            $this->current_item[ $this->current_namespace ][ $el.'#' ] : 0);
            }
            elseif ($this->inchannel) {
                if (!is_null($set)) { 
                    $this->channel[ $this->current_namespace ][ $el.'#' ] = $set; 
                }
            
                $ret = (isset($this->channel[ $this->current_namespace][ $el.'#' ]) ?
                            $this->channel[ $this->current_namespace][ $el.'#' ] : 0);
            }
        }
        else {
            if ( $this->initem ) {
                if (!is_null($set)) { 
                    $this->current_item[ $el.'#' ] = $set; 
                }
            
                $ret = (isset($this->current_item[ $el.'#' ]) ?
                            $this->current_item[ $el.'#' ] : 0);
            }
            elseif ($this->inchannel) {
                if (!is_null($set)) {
                    $this->channel[ $el.'#' ] = $set;
                }
            
                $ret = (isset($this->channel[ $el.'#' ]) ?
                            $this->channel[ $el.'#' ] : 0);
            }
        }
    
        return $ret;
    }

    function normalize_enclosure (&$source, $from, &$dest, $to, $i) {
        $id_from = $this->element_id($from, $i);
        $id_to = $this->element_id($to, $i);

        if (isset($source["{$id_from}@"])) {
            foreach (explode(',', $source["{$id_from}@"]) as $attr) {
                if ($from=='link_enclosure' and $attr=='href') { 
                    # from Atom
                    $dest["{$id_to}@url"] = $source["{$id_from}@{$attr}"];
                    $dest["{$id_to}"] = $source["{$id_from}@{$attr}"];
                }
                elseif ($from=='enclosure' and $attr=='url') { 
                    # from RSS
                    $dest["{$id_to}@href"] = $source["{$id_from}@{$attr}"];
                    $dest["{$id_to}"] = $source["{$id_from}@{$attr}"];
                }
                else {
                    $dest["{$id_to}@{$attr}"] = $source["{$id_from}@{$attr}"];
                }
            }
        }
    }

    function normalize_atom_person (&$source, $person, &$dest, $to, $i) {
        $id = $this->element_id($person, $i);
        $id_to = $this->element_id($to, $i);

        # Atom 0.3 <=> Atom 1.0
        if ($this->feed_version >= 1.0) { 
            $used = 'uri'; $norm = 'url'; 
        }
        else {
            $used = 'url'; $norm = 'uri';
        }

        if (isset($source["{$id}_{$used}"])) {
            $dest["{$id_to}_{$norm}"] = $source["{$id}_{$used}"];
        }

        # Atom to RSS 2.0 and Dublin Core
        # RSS 2.0 person strings should be valid e-mail addresses if possible.
        if (isset($source["{$id}_email"])) {
            $rss_author = $source["{$id}_email"];
        }
        
        if (isset($source["{$id}_name"])) {
            $rss_author = $source["{$id}_name"] . (isset($rss_author) ? " <$rss_author>" : '');
        }
        
        if (isset($rss_author)) {
            $source[$id] = $rss_author;     # goes to top-level author or contributor
            $dest[$id_to] = $rss_author;    # goes to dc:creator or dc:contributor
        }
    }

    # Normalize Atom 1.0 and RSS 2.0 categories to Dublin Core...
    function normalize_category (&$source, $from, &$dest, $to, $i) {
        $cat_id = $this->element_id($from, $i);
        $dc_id = $this->element_id($to, $i);

        # first normalize category elements: Atom 1.0 <=> RSS 2.0
        if ( isset($source["{$cat_id}@term"]) ) { 
            # category identifier
            $source[$cat_id] = $source["{$cat_id}@term"];
        }
        elseif ( $this->feed_type == RSS ) {
            $source["{$cat_id}@term"] = $source[$cat_id];
        }
        
        if ( isset($source["{$cat_id}@scheme"]) ) { // URI to taxonomy
            $source["{$cat_id}@domain"] = $source["{$cat_id}@scheme"];
        } elseif ( isset($source["{$cat_id}@domain"]) ) {
            $source["{$cat_id}@scheme"] = $source["{$cat_id}@domain"];
        }

        // Now put the identifier into dc:subject
        $dest[$dc_id] = $source[$cat_id];
    }
    
    # ... or vice versa
    function normalize_dc_subject (&$source, $from, &$dest, $to, $i) {
        $dc_id = $this->element_id($from, $i);
        $cat_id = $this->element_id($to, $i);

        $dest[$cat_id] = $source[$dc_id];       // RSS 2.0
        $dest["{$cat_id}@term"] = $source[$dc_id];  // Atom 1.0
    }

    # simplify the logic for normalize(). Makes sure that count of elements and
    # each of multiple elements is normalized properly. If you need to mess
    # with things like attributes or change formats or the like, pass it a
    # callback to handle each element.
    
    function normalize_element (&$source, $from, &$dest, $to, $via = NULL) {
        if (isset($source[$from]) or isset($source["{$from}#"])) {
            if (isset($source["{$from}#"])) {
                $n = $source["{$from}#"];
                $dest["{$to}#"] = $source["{$from}#"];
            }
            else { 
                $n = 1;
            }

            for ($i = 1; $i <= $n; $i++) {
                if (isset($via)) { // custom callback for ninja attacks
                    $this->{$via}($source, $from, $dest, $to, $i);
                }
                else { // just make it the same
                    $from_id = $this->element_id($from, $i);
                    $to_id = $this->element_id($to, $i);
                    $dest[$to_id] = $source[$from_id];
                }
            }
        }
    }

    function normalize () {
        // if atom populate rss fields and normalize 0.3 and 1.0 feeds
        
        if ( $this->is_atom() ) {
            // Atom 1.0 elements <=> Atom 0.3 elements (Thanks, o brilliant wordsmiths of the Atom 1.0 standard!)
            if ($this->feed_version < 1.0) {
                $this->normalize_element($this->channel, 'tagline', $this->channel, 'subtitle');
                $this->normalize_element($this->channel, 'copyright', $this->channel, 'rights');
                $this->normalize_element($this->channel, 'modified', $this->channel, 'updated');
            }
            else {
                $this->normalize_element($this->channel, 'subtitle', $this->channel, 'tagline');
                $this->normalize_element($this->channel, 'rights', $this->channel, 'copyright');
                $this->normalize_element($this->channel, 'updated', $this->channel, 'modified');
            }
        
            $this->normalize_element($this->channel, 'author', $this->channel['dc'], 'creator', 'normalize_atom_person');
            $this->normalize_element($this->channel, 'contributor', $this->channel['dc'], 'contributor', 'normalize_atom_person');

            // Atom elements to RSS elements
            $this->normalize_element($this->channel, 'subtitle', $this->channel, 'description');
        
            if ( isset($this->channel['logo']) ) {
                $this->normalize_element($this->channel, 'logo', $this->image, 'url');
                $this->normalize_element($this->channel, 'link', $this->image, 'link');
                $this->normalize_element($this->channel, 'title', $this->image, 'title');
            }

            for ( $i = 0; $i < count($this->items); $i++) {
                $item = $this->items[$i];

                // Atom 1.0 elements <=> Atom 0.3 elements
                if ($this->feed_version < 1.0) {
                    $this->normalize_element($item, 'modified', $item, 'updated');
                    $this->normalize_element($item, 'issued', $item, 'published');
                } else {
                    $this->normalize_element($item, 'updated', $item, 'modified');
                    $this->normalize_element($item, 'published', $item, 'issued');
                }

                // "If an atom:entry element does not contain
                // atom:author elements, then the atom:author elements
                // of the contained atom:source element are considered
                // to apply. In an Atom Feed Document, the atom:author
                // elements of the containing atom:feed element are
                // considered to apply to the entry if there are no
                // atom:author elements in the locations described
                // above." <http://atompub.org/2005/08/17/draft-ietf-atompub-format-11.html#rfc.section.4.2.1>
                if (!isset($item["author#"])) {
                    if (isset($item["source_author#"])) { // from aggregation source
                        $source = $item;
                        $author = "source_author";
                    } elseif (isset($this->channel["author#"])) { // from containing feed
                        $source = $this->channel;
                        $author = "author";
                    }

                    $item["author#"] = $source["{$author}#"];
                    
                    for ($au = 1; $au <= $item["author#"]; $au++) {
                        $id_to = $this->element_id('author', $au);
                        $id_from = $this->element_id($author, $au);
                    
                        $item[$id_to] = $source[$id_from];
                    
                        foreach (array('name', 'email', 'uri', 'url') as $what) {
                            if (isset($source["{$id_from}_{$what}"])) {
                                $item["{$id_to}_{$what}"] = $source["{$id_from}_{$what}"];
                            }
                        }
                    }
                }

                // Atom elements to RSS elements
                $this->normalize_element($item, 'author', $item['dc'], 'creator', 'normalize_atom_person');
                $this->normalize_element($item, 'contributor', $item['dc'], 'contributor', 'normalize_atom_person');
                $this->normalize_element($item, 'summary', $item, 'description');
                $this->normalize_element($item, 'atom_content', $item['content'], 'encoded');
                $this->normalize_element($item, 'link_enclosure', $item, 'enclosure', 'normalize_enclosure');

                // Categories
                if ( isset($item['category#']) ) { 
                    # Atom 1.0 categories to dc:subject and RSS 2.0 categories
                    $this->normalize_element($item, 'category', $item['dc'], 'subject', 'normalize_category');
                }
                elseif ( isset($item['dc']['subject#']) ) { 
                    # dc:subject to Atom 1.0 and RSS 2.0 categories
                    $this->normalize_element($item['dc'], 'subject', $item, 'category', 'normalize_dc_subject');
                }

                // Normalized item timestamp
                $atom_date = (isset($item['published']) ) ? $item['published'] : $item['updated'];
            
                if ( $atom_date ) {
                    $epoch = @parse_w3cdtf($atom_date);
                    
                    if ($epoch and $epoch > 0) {
                        $item['date_timestamp'] = $epoch;
                    }
                }

                $this->items[$i] = $item;
            }
        }
        elseif ( $this->is_rss() ) {
            // RSS elements to Atom elements
            $this->normalize_element($this->channel, 'description', $this->channel, 'tagline'); // Atom 0.3
            $this->normalize_element($this->channel, 'description', $this->channel, 'subtitle'); // Atom 1.0 (yay wordsmithing!)
            $this->normalize_element($this->image, 'url', $this->channel, 'logo');

            for ( $i = 0; $i < count($this->items); $i++) {
                $item = $this->items[$i];
        
                // RSS elements to Atom elements
                $this->normalize_element($item, 'description', $item, 'summary');
                $this->normalize_element($item['content'], 'encoded', $item, 'atom_content');
                $this->normalize_element($item, 'enclosure', $item, 'link_enclosure', 'normalize_enclosure');

                // Categories
                if ( isset($item['category#']) ) { 
                    # RSS 2.0 categories to dc:subject and Atom 1.0 categories
                    $this->normalize_element($item, 'category', $item['dc'], 'subject', 'normalize_category');
                }
                elseif ( isset($item['dc']['subject#']) ) { 
                    # dc:subject to Atom 1.0 and RSS 2.0 categories
                    $this->normalize_element($item['dc'], 'subject', $item, 'category', 'normalize_dc_subject');
                }

                // Normalized item timestamp
                if ( $this->is_rss() == '1.0' and isset($item['dc']['date']) ) {
                    $epoch = @parse_w3cdtf($item['dc']['date']);
                    
                    if ($epoch and $epoch > 0) {
                        $item['date_timestamp'] = $epoch;
                    }
                }
                elseif ( isset($item['pubdate']) ) {
                    $epoch = @strtotime($item['pubdate']);
                    if ($epoch > 0) {
                        $item['date_timestamp'] = $epoch;
                    }
                }

                $this->items[$i] = $item;
            }
        }
    }
    
    
    function is_rss () {
        if ( $this->feed_type == RSS ) {
            return $this->feed_version; 
        }
        else {
            return false;
        }
    }
    
    function is_atom() {
        if ( $this->feed_type == ATOM ) {
            return $this->feed_version;
        }
        else {
            return false;
        }
    }

    #
    # return XML parser, and possibly re-encoded source
    #
    
    function create_parser($source, $out_enc, $in_enc, $detect) {
        if ( substr(phpversion(),0,1) == 5) {
            $parser = $this->php5_create_parser($in_enc, $detect);
        }
        else {
            list($parser, $source) = $this->php4_create_parser($source, $in_enc, $detect);
        }
        if ($out_enc) {
            $this->encoding = $out_enc;
            xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, $out_enc);
        }
        
        return array($parser, $source);
    }
    
    #
    # Instantiate an XML parser under PHP5
    #
    # PHP5 will do a fine job of detecting input encoding
    # if passed an empty string as the encoding. 
    #
    # All hail libxml2!
    #
    
    function php5_create_parser($in_enc, $detect) {
        
        # by default php5 does a fine job of detecting input encodings
        if(!$detect && $in_enc) {
            return xml_parser_create($in_enc);
        }
        else {
            return xml_parser_create('');
        }
    }
    
    #
    # Instaniate an XML parser under PHP4
    #
    # Unfortunately PHP4's support for character encodings
    # and especially XML and character encodings sucks.  As
    # long as the documents you parse only contain characters
    # from the ISO-8859-1 character set (a superset of ASCII,
    # and a subset of UTF-8) you're fine.  However once you
    # step out of that comfy little world things get mad, bad,
    # and dangerous to know.
    #
    # The following code is based on SJM's work with FoF
    # @see http://minutillo.com/steve/weblog/2004/6/17/php-xml-and-character-encodings-a-tale-of-sadness-rage-and-data-loss
    #
    
    function php4_create_parser($source, $in_enc, $detect) {
        if ( !$detect ) {
            return array(xml_parser_create($in_enc), $source);
        }
        
        if (!$in_enc) {
            if (preg_match('/<?xml.*encoding=[\'"](.*?)[\'"].*?>/m', $source, $m)) {
                $in_enc = strtoupper($m[1]);
                $this->source_encoding = $in_enc;
            }
            else {
                $in_enc = 'UTF-8';
            }
        }
        
        if ($this->known_encoding($in_enc)) {
            return array(xml_parser_create($in_enc), $source);
        }
        
        # the dectected encoding is not one of the simple encodings PHP knows
        
        # attempt to use the iconv extension to
        # cast the XML to a known encoding
        # @see http://php.net/iconv
       
        if (function_exists('iconv'))  {
            $encoded_source = iconv($in_enc,'UTF-8', $source);
            if ($encoded_source) {
                return array(xml_parser_create('UTF-8'), $encoded_source);
            }
        }
        
        # iconv didn't work, try mb_convert_encoding
        # @see http://php.net/mbstring
        
        if( function_exists('mb_convert_encoding')) {
            $encoded_source = mb_convert_encoding($source, 'UTF-8', $in_enc );
            if ($encoded_source) {
                return array(xml_parser_create('UTF-8'), $encoded_source);
            }
        }
        
        #
        # else 
        #
        
        $this->error("Feed is in an unsupported character encoding. ($in_enc) " .
                     "You may see strange artifacts, and mangled characters.",
                     E_USER_NOTICE);
            
        return array(xml_parser_create(), $source);
    }
    
    function known_encoding($enc) {
        $enc = strtoupper($enc);
        if ( in_array($enc, $this->_KNOWN_ENCODINGS) ) {
            return $enc;
        }
        else {
            return false;
        }
    }

    function error ($errormsg, $lvl=E_USER_WARNING) {
        // append PHP's error message if track_errors enabled
        if ( isset($php_errormsg) ) { 
            $errormsg .= " ($php_errormsg)";
        }
        if ( MAGPIE_DEBUG ) {
            trigger_error( $errormsg, $lvl);        
        }
        else {
            error_log( $errormsg, 0);
        }
        
        $notices = E_USER_NOTICE|E_NOTICE;
        if ( $lvl&$notices ) {
            $this->WARNING = $errormsg;
        } else {
            $this->ERROR = $errormsg;
        }
    }

    // magic ID function for multiple elemenets.
    // can be called as static MagpieRSS::element_id()
    function element_id ($el, $counter) {
        return $el . (($counter > 1) ? '#'.$counter : '');
    }
} // end class RSS

function map_attrs($k, $v) {
    return "$k=\"$v\"";
}

// patch to support medieval versions of PHP4.1.x, 
// courtesy, Ryan Currie, ryan@digibliss.com

if (!function_exists('array_change_key_case')) {
    define("CASE_UPPER",1);
    define("CASE_LOWER",0);


    function array_change_key_case($array,$case=CASE_LOWER) {
        if ($case==CASE_LOWER) {
            $cmd='strtolower';
        }
        elseif ($case==CASE_UPPER) {
            $cmd='strtoupper';
        }

        foreach($array as $key=>$value) {
            $output[$cmd($key)]=$value;
        }

        return $output;
    }
}
