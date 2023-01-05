let menu = document.getElementById('menu')
let x = document.getElementById('close')

menu.addEventListener('click', () => {
    document.getElementById('navbar').style.display = 'block'
    document.getElementById('content').style.display = 'none'
    menu.style.display = 'none'
})

x.addEventListener('click', () => {
    menu.style.display = 'block'
    document.getElementById('navbar').style.display = 'none'
    document.getElementById('content').style.display = 'flex'
})