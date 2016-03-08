var image_array = [];
var current_index = 0;

function reader_onload() {
	var a_next_image = document.getElementById("next-image");
	var a_prev_image = document.getElementById("prev-image");

	a_next_image.onclick = function() { next_image(); };
	a_prev_image.onclick = function() { prev_image(); };


	var div_image_list = document.getElementById("image-list");
	if (div_image_list != null) {
		// populate the image array
		var image_url = div_image_list.firstElementChild;
		while (image_url != null) {
			image_array.push(image_url.textContent);

			image_url = image_url.nextElementSibling;
		}

		var img = document.createElement("img");
		img.setAttribute("src", image_array[current_index]);
		img.setAttribute("id", "img-current");

		var div_current_image =document.getElementById("current-image");
		div_current_image.appendChild(img);
	}

	window.scrollTo(0,document.body.scrollHeight);
}

function next_image() {
	if (current_index + 1 < image_array.length) {
		var image_current = document.getElementById("img-current");
		image_current.setAttribute("src", image_array[++current_index]);

		var file_index = document.getElementById("file-index");
		file_index.innerText = parseInt(file_index.innerText) + 1;
	}
}

function prev_image() {
	if (current_index - 1 >= 0) {
		var image_current = document.getElementById("img-current");
		image_current.setAttribute("src", image_array[--current_index]);

		var file_index = document.getElementById("file-index");
		file_index.innerText = parseInt(file_index.innerText) - 1;
	}
}