const $fileUpload = document.querySelector('input#fileUploadInput')
const $fileUploadSpan = document.querySelector('#fileUpload span')

document.addEventListener('DOMContentLoaded', fetchFileList)

$fileUpload.addEventListener('change', e => {
	const file = $fileUpload?.files?.[0]

	if (!file || !file.name) return

	$fileUploadSpan.innerText = file.name
})

function prepareFileUpload() {
	$fileUpload.click()
}

function handleFileUpload() {
	const file = $fileUpload?.files?.[0]
	let requestStatus

	if (!file) {
		notify('No file selected', 'info')
		return
	}

	fetch('/download', {
		method: 'PUT',
		headers: {
			'Content-Type': file.type || 'application/octet-stream',
			'X-Filename': file.name
		},
		body: file
	})
		.then(res => {
			requestStatus = res.status
			return res.text()
		})
		.then(res => {
			if (requestStatus === 200) {
				$fileUploadSpan.innerText = '...'
				$fileUpload.value = ''
				fetchFileList()
				notify('File uploaded successfully', 'success')
				return
			} else notify(requestStatus, 'error')
		})
		.catch(err => console.error('Upload failed:', err))
}

function fetchFileList() {
	fetch('/download')
		.then(res => res.text())
		.then(res => {
			const parser = new DOMParser()
			const doc = parser.parseFromString(res, 'text/html')
			const listingDiv = doc.querySelector('div.listing')
			const $myDrive = document.querySelector('my-drive')

			$myDrive.innerHTML = ''
			$myDrive.innerHTML = listingDiv.outerHTML
		})
}
