// script.js
// by Dr.D.
// last updated: 2024-08-04

// Hide/show but maintain the spacing.
const show = elem => {
    elem.style.visibility = 'visible'
}
const hide = elem => {
    elem.style.visibility = 'hidden'
}
const toggleHide = elem => {
    // if the element is visible, hide it
    if (window.getComputedStyle(elem).visibility !== 'hidden') {
        hide(elem)
        return
    }

    // show the element
    show(elem)
}

// Hide/show, which removes the element
const rm = elem => {
    elem.style.display = 'none'
}
const mk = elem => {
    elem.style.display = 'block'
}
const toggleDisplay = elem => {
    // if the element is visible, hide it
    if (window.getComputedStyle(elem).display !== 'none') {
        rm(elem)
        return
    }

    // show the element
    mk(elem)
}

// Set the original state of elements
hide(document.querySelector('#VM'))
rm(document.querySelector('#NES'))

// Event listeners to navigation links
document.getElementById('li-vm').addEventListener(
    'click',
    function (e) {
        // Add section back to DOM; remove others, toggle vis.
        mk(document.querySelector('#VM'));
        rm(document.querySelector('#NES'));
        toggleHide(document.querySelector('#VM'));
    }
);
document.getElementById('li-nes').addEventListener(
    'click',
    function (e) {
        // Remove welcome text, toggle vis.
        hide(document.querySelector('#VM'));
        rm(document.querySelector('#VM'));
        toggleDisplay(document.querySelector('#NES'));
    }
);