

const registerServiceWorker = async () => {
    if ('serviceWorker' in navigator) {
        try {
            const registration = navigator.serviceWorker.register("/service_worker.js", {
                scope: "/",
            })
        }
        catch (err) {
            err => console.error("service worker not registered", err)
        }
    }
}
registerServiceWorker()
