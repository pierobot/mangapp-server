// http://stackoverflow.com/a/9609450
var decodeEntities = (function() {
  // this prevents any overhead from creating the object each time
  var element = document.createElement('div');

  String.fromHtmlEntities = function (str) {
    if(str && typeof str === 'string') {
      // strip script/html tags
      str = str.replace(/<script[^>]*>([\S\s]*?)<\/script>/gmi, '');
      str = str.replace(/<\/?\w(?:[^"'>]|"[^"]*"|'[^']*')*>/gmi, '');
      element.innerHTML = str;
      str = element.textContent;
      element.textContent = '';
    }

    return str;
  }
})();

function decode_html_entities() {
    var description = document.getElementById("description");
    if (description != null) {
        var description_text = description.firstElementChild.nextElementSibling;
        description_text.textContent = String.fromHtmlEntities(description_text.textContent);
    } 

    var assoc_names = document.getElementById("assoc-names");
    if (assoc_names != null) {
        var name = assoc_names.firstElementChild;
        while (name != null) {
            name.textContent = String.fromHtmlEntities(name.textContent);
            name = name.nextElementSibling;
        }
    }
}