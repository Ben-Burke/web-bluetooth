// This javascript module implements the cigarette counter functionality as per cigarette_counter.html

// This function is called when the page is loaded
function init() {
    // Get the current count from the local storage
    var count = localStorage.getItem("cigarette_count");
    // If the count is null, set it to 0
    if (count == null) {
        count = 0;
    }
    // Set the count in the text box
    document.getElementById("cigarette_count").value = count;
}

// This function is called when the user clicks the "Add" button
function addCigarette() {
    // Get the current count from the text box
    var count = document.getElementById("cigarette_count").value;
    // Increment the count
    count++;
    // Set the count in the text box
    document.getElementById("cigarette_count").value = count;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", count);
}

// This function is called when the user clicks the "Reset" button
function resetCigarette() {
    // Set the count in the text box to 0
    document.getElementById("cigarette_count").value = 0;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", 0);
}

// This function is called when the user clicks the "Subtract" button
function subtractCigarette() {
    // Get the current count from the text box
    var count = document.getElementById("cigarette_count").value;
    // Decrement the count
    count--;
    // Set the count in the text box
    document.getElementById("cigarette_count").value = count;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", count);
}

// This function is called when the user clicks the "Add" button
function addCigarette() {
    // Get the current count from the text box
    var count = document.getElementById("cigarette_count").value;
    // Increment the count
    count++;
    // Set the count in the text box
    document.getElementById("cigarette_count").value = count;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", count);
}

// This function is called when the user clicks the "Reset" button
function resetCigarette() {
    // Set the count in the text box to 0
    document.getElementById("cigarette_count").value = 0;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", 0);
}

// This function is called when the user clicks the "Subtract" button
function subtractCigarette() {
    // Get the current count from the text box
    var count = document.getElementById("cigarette_count").value;
    // Decrement the count
    count--;
    // Set the count in the text box
    document.getElementById("cigarette_count").value = count;
    // Save the count in the local storage
    localStorage.setItem("cigarette_count", count);
}

