const featureDetails = document.querySelectorAll('.feature-detail');

const observer = new IntersectionObserver((entries) => {
	entries.forEach(entry => {
		if (entry.isIntersecting) {
			entry.target.classList.add('active');
			const img = entry.target.querySelector('img');
			img.classList.add('active');
		}
		else {
			entry.target.classList.remove('active');
			const img = entry.target.querySelector('img');
			img.classList.remove('active');
		}
	});
}, {
	threshold: 0.1
});

featureDetails.forEach(detail => {
	observer.observe(detail);
});

function updateContent(langData) {
	document.querySelectorAll('[data-i18n]').forEach(element => {
		const key = element.getAttribute('data-i18n');

		if (key in langData) {
			if (element.tagName === "IMG") {
				element.src = langData[key];
			} else {
				element.innerHTML = langData[key];
			}
		} else {
			console.warn(`Missing translation for key: '${key}'`);
		}
	});
}

function setLanguagePreference(lang) {
	localStorage.setItem('language', lang);
	location.reload();
}

async function fetchLanguageData(lang) {
	try {
		const response = await fetch(`assets/lang/${lang}.json`);
		if (!response.ok) {
			throw new Error(`Language file for '${lang}' not found`);
		}
		return response.json();
	} catch (error) {
		console.warn(error.message + `. Falling back to English.`);
		return fetchLanguageData('en');
	}
}

async function detectAndLoadLanguage() {
	const savedLanguage = localStorage.getItem('language');
	const browserLanguage = navigator.language || navigator.languages[0];
	const languageToLoad = savedLanguage || browserLanguage.split('-')[0] || 'en';

	console.log(`Detected language: ${browserLanguage}. Loading: ${languageToLoad}`);

	const langData = await fetchLanguageData(languageToLoad);
	updateContent(langData);
}

async function changeLanguage(lang) {
	setLanguagePreference(lang);
}

window.addEventListener('DOMContentLoaded', detectAndLoadLanguage);

function toggleDropdown() {
	const dropdownMenu = document.getElementById('dropdownMenu');
	dropdownMenu.style.display = dropdownMenu.style.display === 'block' ? 'none' : 'block';
}

window.onclick = function (event) {
	const dropdownMenu = document.getElementById('dropdownMenu');
	const dropdownButton = document.querySelector('.header-button');
	if (!dropdownMenu.contains(event.target) && event.target !== dropdownButton) {
		dropdownMenu.style.display = 'none';
	}
};

function downloadFennix() {
	// TODO: Here will download the iso file
	window.location.href = "https://github.com/EnderIce2/Fennix";
}
