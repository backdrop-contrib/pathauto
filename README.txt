The pathauto module provides support functions for other modules to 
automatically generate aliases based on appropriate criteria, with a 
central settings path for site administrators.

Implementations are provided for nodes, taxonomy terms, users, and 
blogs.


Benefits

Besides making the page address more reflective of its content than
"node/138", it's important to know that modern search engines give 
heavy weight to search terms which appear in a page's URL. By 
automatically using keywords based directly on the page content in the URL, 
relevant search engine hits for your page can be significantly
enhanced.


Installation

Note that pathauto is an extension to the path module, which must
be enabled.

1. Unpack the pathauto folder and contents in the modules directory of 
your Drupal installation.
2. Enable the pathauto module in the administration tools.
3. If you're not using Drupal's default administrative account, make
sure "administer pathauto" is enabled through access control administration.
4. Visit the pathauto settings page to define the desired alias
patterns.


Implementing automatic aliases for a module

There are three pieces a module must provide to support automatic 
aliasing (see pathauto_node.inc, pathauto_taxonomy.inc, and
pathauto_user.inc for examples). Ideally this code will be integrated
into a module, but a developer may extend a module they don't maintain
by implementing these functions in a file of the form
pathauto_<module>.inc in the pathauto directory.

1 - Settings hook

You must implement hook_pathauto($op), where $op is always (at this
time) 'settings'. Return an object (NOT an array) containing the
following members, which will be used by pathauto to build a group
of settings for your module and define the variables for saving your
settings:

module - The name of your module (e.g., 'node')
groupheader - The translated label for the settings group (e.g.,
  t('Node path settings')
patterndescr - The translated label for the default pattern (e.g.,
  t('Default path pattern (applies to all node types with blank patterns below)')
patterndefault - A translated default pattern (e.g., t('[cat]/[title].html'))
placeholders - An array whose keys consist of the translated placeholders
  which will appear in patterns (e.g., t('[title]')) and values are 
  the translated description of the placeholders (e.g.,
  t('The title of the node, with spaces and punctuation.')
patternitems - For modules which need to express multiple patterns
  (for example, the node module supports a separate pattern for each
  node type), an array whose keys consist of identifiers for each
  pattern (e.g., the node type name) and values consist of the
  translated label for the pattern
bulkname - For modules which support a bulk update operation, the
  translated label for the action (e.g., t('Bulk update node paths'))
bulkdescr - For modules which support a bulk update operation, a 
  translated, more thorough description of what the operation will do 
  (e.g., t('Generate aliases for all existing nodes which do not already have aliases.'))

2 - $alias = pathauto_create_alias($module, $placeholders, $src, $type=NULL)

At the appropriate time (usually when a new item is being created for
which a generated alias is desired), call pathauto_create_alias() to
generate and create the alias.

module - The name of your module (e.g., 'node')
$placeholders - An array whose keys consist of the translated placeholders
  which appear in patterns (e.g., t('[title]')) and values are the 
  actual values to be substituted into the pattern (e.g., $node->title)
$src - The "real" URI of the content to be aliased (e.g., "node/$node->nid")
$type - For modules which provided patternitems in hook_autopath(),
  the relevant identifier for the specific item to be aliased (e.g.,
  $node->type)

pathauto_create_alias() returns the alias that was created.

3 - Bulk update function

If a module supports bulk updating of aliases, it must provide a
function of this form, to be called by pathauto when the corresponding
checkbox is selected and the settings page submitted:

function <module>_pathauto_bulkupdate()

The function should iterate over the content items controlled by the
module, calling pathauto_create_alias() for each one. It is
recommended that the function report on its success (e.g., with a 
count of created aliases) via drupal_set_message().
  
  
Credits

The original module combined the functionality of Mike Ryan's autopath with
Tommy Sundstrom's path_automatic.

Significant enhancements have been contributed by jdmquin@www.bcdems.net.

Current maintainer: Mike Ryan (drupal@virtuoso-performance.com)