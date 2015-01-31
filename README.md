Pathauto
========

The Pathauto module provides support functions for other modules to
automatically generate aliases based on appropriate criteria, with a
central settings path for site administrators.

Implementations are provided for core entity types: content, taxonomy terms,
and users (including blogs and tracker pages).

Pathauto also provides a way to delete large numbers of aliases.  This feature
is available at  Administer > Site building > URL aliases > Delete aliases

Benefits
--------

Besides making the page address more reflective of its content than
"node/138", it's important to know that modern search engines give
heavy weight to search terms which appear in a page's URL. By
automatically using keywords based directly on the page content in the URL,
relevant search engine hits for your page can be significantly
enhanced.

*Important note*: This project is targeted for inclusion in Backdrop 1.1.0, due
May 15, 2015. It is likely that after upgrading to Backdrop 1.1.0, you will no
longer need this module.

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

- This module has no settings page, though it does modify a number of forms
  throughout core to include a link to the token browser.


Notices
-------

Pathauto just adds URL aliases to content, users, and taxonomy terms.
Because it's an alias, the standard Backdrop URL (for example node/123 or
taxonomy/term/1) will still function as normal.  If you have external links
to your site pointing to standard Backdrop URLs, or hardcoded links in a module,
template, content or menu which point to standard Backdrop URLs it will bypass
the alias set by Pathauto.

There are reasons you might not want two URLs for the same content on your
site. If this applies to you, please note that you will need to update any
hard coded links in your content or blocks.

If you use the "system path" (i.e. node/10) for menu items and settings like
that, Backdrop will replace it with the url_alias.

For external links, you might want to consider the Path Redirect or
Global Redirect modules, which allow you to set forwarding either per item or
across the site to your aliased URLs.

### URLs (not) Getting Replaced With Aliases:

Please bear in mind that only URLs passed through Backdrop's l() or url()
functions will be replaced with their aliases during page output. If a module
or your template contains hardcoded links, such as 'href="node/$node->nid"'
those won't get replaced with their corresponding aliases. Use the
Backdrop API instead:

- `'href="'. url("node/$node->nid") .'"'` or
- `l("Your link title", "node/$node->nid")`

See the documentation for url() and l() for more information:

https://api.backdropcms.org/api/backdrop/core!includes!common.inc/function/url
https://api.backdropcms.org/api/backdrop/core!includes!common.inc/function/l

### Disabling Pathauto for a specific content type (or taxonomy)

When the pattern for a content type is left blank, the default pattern will be
used. But if the default pattern is also blank, Pathauto will be disabled
for that content type.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Current Maintainers
-------------------

- Eduardo Garcia (https://github.com/enzolutions/)
- Nate Haug (https://github.com/quicksketch/)

Credits
-------

Ported by Backdrop by Eduardo Garcia.

This module was originally written and maintained for Drupal by:

- Dave Reid - http://www.davereid.net
- Greg Knaddison - http://www.knaddison.com
- Mike Ryan - http://mikeryan.name
- Frederik 'Freso' S. Olesen - http://freso.dk

The original module combined the functionality of Mike Ryan's autopath with
Tommy Sundstrom's path_automatic.

Significant enhancements were contributed by jdmquin @ www.bcdems.net.

Other suggestions and patches contributed by the Drupal community.
