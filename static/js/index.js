

function list_initialize_events() {
	var manga_list = document.getElementById("list-manga");
	
	if (manga_list != null) {
		// Setup the div's onclick events
		var div = manga_list.firstElementChild;
		while (div != null) {
			div.onclick = function (e) {
				window.open("/mangapp/details/" + this.id, "_blank");
			};

			// Setup the list item's mouse events to get pretty colors :^)
			var li = div.firstElementChild;
			li.onmouseover = function(e) {
				this.setAttribute("style", "background-color: #e9e9e9");
			};
			li.onmouseout = function(e) {
				this.setAttribute("style", "background-color: #ffffff");
			};
			li.onmousedown = function(e) {
				this.setAttribute("style", "background-color: #265a88");
			};
			li.onmouseup = function(e) {
				this.setAttribute("style", "background-color: #ffffff");
			};

			div = div.nextElementSibling;
		}
	}
}