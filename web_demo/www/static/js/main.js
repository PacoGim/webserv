const colorRed = '#dc143c'
const colorBlue = '#6495ed'
const colorGreen = '#00cc00'
const colorGold = '#FFD700'
const host = '/'

const $customRequest = document.querySelector('section#customRequest')
const $customRequestSendButton = $customRequest.querySelector('button.submit')

registerEvents()

function getColorFromStatusCode(statusCode) {
	if (statusCode === 418) return colorGold
	if (statusCode >= 200 && statusCode < 300) return colorGreen
	if (statusCode >= 400 && statusCode < 500) return colorRed
	if (statusCode >= 500 && statusCode < 600) return colorBlue
}

function registerEvents() {
	if (!$customRequest) return
	if (!$customRequestSendButton) return
	$customRequestSendButton.addEventListener('click', sendCustomRequest)
}

function sendCustomRequest() {
	const $method = $customRequest.querySelector('.method')
	if (!$method) return console.log('Select element is undefined')

	const $endpoint = $customRequest.querySelector('.endpoint')
	if (!$endpoint) return console.log('Endpoint element is undefined')

	const $body = $customRequest.querySelector('.body')
	if (!$body) return console.log('Body element is undefined')

	const $statusCode = $customRequest.querySelector('.statusCode')
	if (!$statusCode) return console.log('StatusCode element is undefined')

	const $statusResponse = $customRequest.querySelector('.statusResponse')
	if (!$statusResponse) return console.log('StatusResponse element is undefined')

	const $response = $customRequest.querySelector('.response')
	if (!$response) return console.log('Response element is undefined')

	const $headers = $customRequest.querySelector('.headers')
	if (!$headers) return console.log('Headers element is undefined')

	const methodValue = $method.value
	const endpointValue = $endpoint.value
	const bodyValue = $body.value

	let options = {
		method: methodValue,
		headers: { 'Content-Type': 'text/plain' },
		body: bodyValue
	}

	if (methodValue === 'GET' || methodValue === 'HEAD') {
		delete options.body
		delete options.headers
	}

	if (methodValue === 'PUT') {
		options.headers = options.headers || {}
		options.headers.Filename = 'default.txt'
	}

	let content_type

	console.log(options)
	let debugLine = `${methodValue} ${endpointValue}`
	let now = performance.now()
	fetch(host + endpointValue, options)
		.then(res => {
			debugLine += ` took: ${(performance.now() - now).toFixed(2)}ms to fetch`
			const color = getColorFromStatusCode(res.status)
			let headers = ''

			for (const [key, value] of res.headers.entries()) {
				headers += `${key} : ${value}
`
			}
			content_type = res.headers.get('Content-Type')

			$statusCode.style.color = color
			$statusResponse.style.color = color

			$statusCode.innerText = res.status
			$statusResponse.innerText = res.statusText
			$headers.value = headers
			return res.text()
		})
		.then(body => {
			now = performance.now()
			$response.innerText = body
			debugLine += ` took: ${(performance.now() - now).toFixed(2)}ms to write`
			debugLine += ` ${body.length} bytes`
		})
		.catch(err => {
			const time = performance.now() - now
			debugLine += ` took: ${time.toFixed(2)}ms`
			const color = getColorFromStatusCode(500)
			$statusCode.style.color = color
			$statusResponse.style.color = color
			$statusCode.innerText = 'Fetch error'
			$response.innerText = err.toString()
		})
		.finally(() => writeDebug(debugLine))
}
