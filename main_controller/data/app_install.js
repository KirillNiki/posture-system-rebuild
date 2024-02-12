
let install_event
let install_button = document.getElementById('install_btn')

window.addEventListener('beforeinstallprompt', function (event) {
    event.preventDefault()
    install_event = event
    install_button.style.visibility = 'visible'
})

install_button.addEventListener('click', async function (event) {
    install_event.prompt()
    const { outcome } = await install_event.userChoice

    console.log(`user choice: ${outcome}`)
    install_button.style.visibility = 'hidden'
})
