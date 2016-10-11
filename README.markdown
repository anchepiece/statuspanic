What is StatusPanic?
====================
StatusPanic is a simple PHP system for creating a way to make a [Panic](http://panic.com)-style [Status Board](http://www.panic.com/blog/2010/03/the-panic-status-board/) with as little effort as possible.

Configuration
====================
Configuring StatusPanic is basically the first and last step of getting it running. Currently, to provide data to modules you must edit them manually. In future releases this might be changed, but that's how it works for now. Other options are specified in [JSON](http://en.wikipedia.org/wiki/JSON) format in a file called [config.json](http://github.com/ajb/statuspanic/blob/master/config.json). If you check out this file you can see how it's set up. You can assume that all options are required at this point. Args can be provided to a module via a GET request by adding them to an ARGS array.

Of special note is the 'rotate' option. You may specify this to 'left', 'right', or 'no' to rotate the whole display with CSS. This lets you display it on a vertical monitor on a system that doesn't support such things. This option is experimental and doesn't work very well.

An 'update' of 0 indicates that module doesn't require updates, and thus whatever is loaded the first time is what remains.

How It Works
====================
The modules are loaded on the initial page load. From there, Ajax is used to update each module every x seconds, where x is specified in config.json for each module. You could have some modules refresh every minute and others refresh every week.

Notes
====================
This will currently work best on a WebKit browser such as Safari or Chrome. It will also work on Firefox, but I'm not testing it there very often, so it will always be a couple versions behind.

Have a suggestion? Excellent - let me know.

Have a photo of a status board you made with statuspanic? Excellent - I want to see it!

Example
====================
I've got a working version on my website. It's basically just a direct installation - no editing has been done from the default configuration. It basically lets you see what it does without trying (since that's a stated goal of this project..)
[Example Installation](http://allynbauer.com/software/statuspanic)

To Do
====================
- More complete readme
- Redo format of HTML/CSS
- Fix the vertical centering glitch