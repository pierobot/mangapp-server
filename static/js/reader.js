var image_urls = [];
var current_index = 0;

var images = [];

function reader_onload() {
	var a_next_image = document.getElementById("next-image");
	var a_prev_image = document.getElementById("prev-image");

	a_next_image.onclick = function() { next_image(); };
	a_prev_image.onclick = function() { prev_image(); };

	$('.dropdown-menu li a').click(function() {
		change_image(parseInt(this.text));
		this.scrollIntoView();
	});

	var div_image_list = document.getElementById("image-list");
	if (div_image_list != null) {
		// populate the image array
		var image_url = div_image_list.firstElementChild;
		while (image_url != null) {
			image_urls.push(image_url.textContent);

			image_url = image_url.nextElementSibling;
		}

		var img_current = document.createElement("img");
		img_current.setAttribute("src", image_urls[current_index]);
		img_current.setAttribute("id", "img-current");
		img_current.onload = function() {
			window.scrollTo(document.body.scrollHeight, 0);
		};
		img_current.onclick = function() {
			next_image();
		};

		var div_current_image =document.getElementById("current-image");
		div_current_image.appendChild(img_current);
	}
}

function next_image() {
	if (current_index + 1 < image_urls.length) {
		var image_current = document.getElementById("img-current");
		image_current.setAttribute("src", image_urls[++current_index]);

		var file_index = document.getElementById("file-index");
		file_index.innerText = parseInt(file_index.innerText) + 1;
	}
}

function prev_image() {
	if (current_index - 1 >= 0) {
		var image_current = document.getElementById("img-current");
		image_current.setAttribute("src", image_urls[--current_index]);

		var file_index = document.getElementById("file-index");
		file_index.innerText = parseInt(file_index.innerText) - 1;
	}
}

function change_image(index) {
	var image_current = document.getElementById("img-current");
	current_index = index;
	image_current.setAttribute("src", image_urls[--current_index]);

	var file_index = document.getElementById("file-index");
	file_index.innerText = parseInt(current_index) + 1;
}