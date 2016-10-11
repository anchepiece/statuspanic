What is StatusPanic?
====================
StatusPanic is a simple PHP system for creating a way to make a [Panic](http://panic.com)-style [Status Board](http://www.panic.com/blog/2010/03/the-panic-status-board/) with as little effort as possible. Branched from the [original project](https://github.com/allynbauer/statuspanic), I've added twitter, rss, and github modules, as well as updated the css to slightly more modern practices.

Configuration
====================
Configuring StatusPanic is basically the first and last step of getting it running. Options are specified in [JSON](http://en.wikipedia.org/wiki/JSON) in config.json. There's a config-defaults.json file that gives some idea of how this should be set up. Args can be provided to a module via a GET request by adding them to an ARGS array.

An 'update' of 0 indicates that module doesn't require updates, and thus whatever is loaded the first time is what remains.

How It Works
====================
The modules are loaded on the initial page load. From there, .load() is used to update each module every x seconds, where x is specified in config.json for each module. You could have some modules refresh every minute and others refresh every week. Cheat sheet: 60 seconds is a minute, 3600 is an hour, 86400 a day, and 604800 a week.

Writing a Module
====================
The board looks for modules based on their "type", which should be a folder in the /modules directory. It then loads /modules/%type%/?arg1=foo&arg2=bar for every module defined in the JSON, and, optionally, includes a css file it looks for at /modules/%type%/%type%.css. Although the index is written in PHP, you could theoretically write a module in any language.

Libraries that more than one module might need access to, such as the PHP Magpie RSS library, should be placed in /shared-libraries.

To-dos
====================
+ continue updating the css
+ more modules?