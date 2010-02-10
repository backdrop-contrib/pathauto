// $Id$
(function ($) {

Drupal.behaviors.pathauto = {
  attach: function(context, settings) {
    $("#edit-path-pathauto-perform-alias", context).once("pathauto", function () {
      var path_input = $("#edit-path-alias", context);
      if ($(this, context).size() && $(this, context).attr("checked")) {
        // Disable input and hide its description.
        path_input.attr("disabled", "disabled");
        path_input.parent().find("div.description", context).hide(0);
      }
      $(this, context).bind("click", function() {
        if ($(this, context).attr("checked")) {
          // Auto-alias checked; disable input.
          path_input.attr("disabled", "disabled");
          path_input.parent().find("div.description", context).slideUp('fast');
        }
        else {
          // Auto-alias unchecked; enable input.
          path_input.removeAttr("disabled");
          path_input[0].focus();
          path_input.parent().find("div.description", context).slideDown('fast');
        }
      });
    });
  }
};

})(jQuery);
