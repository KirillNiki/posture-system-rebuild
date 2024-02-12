
const staticDevCache = 'dev-posture-controll-sys-v1'
const assets = [
    '/',
    '/posture-sys',
    '/style.css',
    '/scripts/app.js',
    '/scripts/logic.js',
    '/app_install.js',
    '/service_worker_reg.js',
    '/manifest.json',
    '/icons/logo-192.png',
    '/icons/logo-512.png',
    '/icons/logo.svg',
]

self.addEventListener("install", installEvent => {
    installEvent.waitUntil(
        caches.open(staticDevCache).then(cache => {
            cache.addAll(assets)
        })
    )
})

self.addEventListener("fetch", fetchEvent => {
    fetchEvent.waitUntil(
        caches.open(staticDevCache).then(cache => {
            cache.keys().then(cacheNames => {
                for (var i = 0; i < cacheNames.length; i++) {
                    console.log(cacheNames[i])
                }
            })
        })
    )

    fetchEvent.respondWith(
        caches.match(fetchEvent.request).then(function (response) {
            console.log(fetchEvent.request)
            if (response) return response

            return fetch(fetchEvent.request)
        })
    )
})
