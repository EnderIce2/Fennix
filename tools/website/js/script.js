document.querySelectorAll(".hdrbtn").forEach((b) => {
    b.onmouseleave = (e) => {
        e.target.style.background = "rgb(15, 15, 15, 0.1)";
        e.target.style.borderImage = null;
    };

    b.addEventListener("mousemove", (e) => {
        const rect = e.target.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        e.target.style.background = `radial-gradient(circle at ${x}px ${y}px , rgba(100, 0, 180, 0.5),rgba(15, 15, 15, 0.1) )`;
        e.target.style.borderImage = `radial-gradient(20% 75% at ${x}px ${y}px ,rgba(255, 255, 255, 0.7),rgba(255, 255, 255, 0.1) ) 1 / 1px / 0px stretch `;
    });
});

document.getElementById("year").innerHTML = new Date().getFullYear();
topButton = document.getElementById("backToTopBtn");
window.onscroll = function () { scrollToTop(); };
function scrollToTop() {
    if (document.body.scrollTop > 20 || document.documentElement.scrollTop > 20) {
        topButton.style.display = "block";
    } else {
        topButton.style.display = "none";
    }
}

function topScript() {
    window.scroll({
        top: 0,
        behavior: 'smooth'
    });
}

function CopyToClipboard(text) {
    if (window.clipboardData && window.clipboardData.setData) {
        return window.clipboardData.setData("Text", text);
    }
    else if (document.queryCommandSupported && document.queryCommandSupported("copy")) {
        var textarea = document.createElement("textarea");
        textarea.textContent = text;
        textarea.style.position = "fixed";
        document.body.appendChild(textarea);
        textarea.select();
        try {
            return document.execCommand("copy");
        }
        catch (ex) {
            console.warn("Copy to clipboard failed.", ex);
            return prompt("Copy to clipboard: Ctrl+C, Enter", text);
        }
        finally {
            document.body.removeChild(textarea);
        }
    }
}