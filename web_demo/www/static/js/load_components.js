loadComponent('navbar', 'navbar.html')
loadComponent('footer', 'footer.html')
loadComponent('notification', 'notification.html')
loadComponent('debug', 'debug.html')
if (localStorage.getItem('cookie-checked') !== "true") loadComponent('cookie', 'cookie.html')

function loadComponent(selector, filePath) {
	const $element = document.querySelector('component#' + selector)

	if (!$element) return
	fetch('/static/components/' + filePath)
		.then(res => res.text())
		.then(res => {
			$element.innerHTML = res
			$element.querySelectorAll('script').forEach(oldScript => {
				const newScript = document.createElement('script')
				if (oldScript.src) {
					newScript.src = oldScript.src
				} else {
					newScript.textContent = oldScript.textContent
				}
				document.head.appendChild(newScript)
				oldScript.remove()
			})
		})
}
