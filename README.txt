The pathauto module automatically generates a path alias at node
creation time when no explicit alias is provided by the user.

Benefits

Besides making the page address more reflective of its content than
"node/138", it's important to know that modern search engines give 
heavy weight to search terms which appear in a page's URL. By 
automatically using the title in the URL, relevant search engine hits 
for your page (presuming a well-chosen title) can be significantly
enhanced.

Installation

Simply copy pathauto.module to the modules directory of your Drupal
installation, and enable it in the administration tools.

Note that pathauto is an extension to the path module, which must
be enabled.

Details

Whenever any user of a pathauto-enabled site submits a new node
which has a title but no path alias, pathauto creates an alias from 
the title (and other node information) as follows:

o The title is lower-cased
o Apostrophes are stripped
o Any sequence of one or more non-alphanumeric characters is replaced
  by a designated separator.
o A pattern is applied which may include taxonomy terms, timestamp
  information, and other information related to the node.
o If the resulting alias duplicates an existing one, a sequence number 
  is appended to generate a unique alias.

Aliases may also automatically be generated for new taxonomy terms
as well.

Settings

A set of placeholders is provided for building an alias pattern - this
can be used to generate aliases in forms such as 
"vocabulary_name/parent_category/category/title" or
"year/month/day/title".

Aliases may also be generated for feeds.

The settings page offers the option of automatically generating aliases 
for all existing unaliased nodes.

Credits

This module combines the functionality of Mike Ryan's autopath with
Tommy Sundstrom's path_automatic.

Support for node-specific patterns and taxonomy term aliasing by 
jdmquin@www.bcdems.net.
