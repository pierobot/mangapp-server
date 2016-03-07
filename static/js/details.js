String.fromHtmlEntities = function(string) {
    return (string+"").replace(/&#\d+;/gm,function(s) {
        return String.fromCharCode(s.match(/\d+/gm)[0]);
    })
};

function decode_html_entities() {
    var assoc_names = document.getElementById("assoc-names");

    if (assoc_names != null) {
        var name = assoc_names.firstElementChild;
        while (name != null) {
            name.textContent = String.fromHtmlEntities(name.textContent);

            name = name.nextElementSibling;
        }
    }
}