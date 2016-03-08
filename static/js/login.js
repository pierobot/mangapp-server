function submit_credentials() {
    var username = document.getElementById("user-input").value;
    var password = document.getElementById("password-input").value;

    $.ajax({
        url: "/mangapp/login",
        method: "GET",
        data: { "username": username, "password": password },
        dataType: "application/json",
        statusCode: {
            200: function() {
                window.location = "/mangapp";
            }
        }
    });
}
