function updateNotificationsPosition() {
	const $notificationWrapper = document.querySelector('notification-wrapper')
	const $containers = $notificationWrapper.querySelectorAll('notification-container')
	const list = Array.from($containers).reverse()

	list.forEach(($container, index) => {
		const offset = index * 50
		$container.style.transform = `translateY(${offset}px)`
	})
}

function notify(msg, level) {
	const $notificationWrapper = document.querySelector('notification-wrapper')
	const $container = document.createElement('notification-container')
	const $notification = document.createElement('notification')
	const $span = document.createElement('span')
	const $closeBtn = document.createElement('close-btn')

	$notification.setAttribute('type', level)
	$span.innerText = msg
	$closeBtn.innerText = 'x'
	$closeBtn.onclick = removeNotification

	$notification.appendChild($span)
	$notification.appendChild($closeBtn)
	$container.appendChild($notification)
	$notificationWrapper.appendChild($container)

	requestAnimationFrame(() => {
		$container.dataset.enabled = 'true'
		updateNotificationsPosition()
	})

	setTimeout(() => {
		removeNotification($container)
	}, 5000)
}

function removeNotification(evt) {
	let $container
	if (evt.tagName === 'NOTIFICATION-CONTAINER') $container = evt
	else $container = evt.target.closest('notification-container')
	$container.dataset.enabled = 'false'

	setTimeout(() => {
		$container.remove()
		updateNotificationsPosition()
	}, 300)
}
