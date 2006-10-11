$Id$

**Description:
The pathauto module provides support functions for other modules to 
automatically generate aliases based on appropriate criteria, with a 
central settings path for site administrators.

Implementations are provided for core content types: nodes, taxonomy 
terms, users, blogs, and events.

**Benefits:

Besides making the page address more reflective of its content than
"node/138", it's important to know that modern search engines give 
heavy weight to search terms which appear in a page's URL. By 
automatically using keywords based directly on the page content in the URL, 
relevant search engine hits for your page can be significantly
enhanced.


**Installation:

Note that pathauto is an extension to the path module, which must
be enabled.

1. Unpack the pathauto folder and contents in the modules directory of 
your Drupal installation.
2. Enable the pathauto module in the administration tools.
3. If you're not using Drupal's default administrative account, make
sure "administer pathauto" is enabled through access control administration.
4. Visit the pathauto settings page:
  When first installing pathauto, to define the desired alias patterns.
  When installing a new version of pathauto, to have any necessary
    database and variable upgrades applied.

**Notice

Please bear in mind that only URLs passed through Drupal's l() or url()
functions will be replaced with their aliases during page output. If a module
or your template contains hardcoded links, such as 'href="node/$node->nid"'
those won't get replaced with their corresponding aliases. Use instead

* 'href="'. url("node/$node->nid") .'"' or
* l("Your link title", "node/$node->nid")

See http://api.drupal.org/api/HEAD/function/url and 
http://api.drupal.org/api/HEAD/function/l for more information.


**Credits:

The original module combined the functionality of Mike Ryan's autopath with
Tommy Sundstrom's path_automatic.

Significant enhancements were contributed by jdmquin @ www.bcdems.net.

Matt England added the tracker support.

Other suggestions and patches contributed by the Drupal community.

Current maintainer: Greg Knaddison (greg AT knaddison DOT com)

