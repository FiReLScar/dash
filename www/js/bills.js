let billsDiv = document.getElementById("bills");

(async () => {
    let billsRaw = await fetch("/api/bills/get")
    let lines = await (await billsRaw.text()).split('\n')
    for (let i = 0; i < lines.length; i++) {
        let line = lines[i]
        if (line.length == 0) continue
        let name = line.split(',')[0]
        let price = line.split(',')[1]
        let freq = line.split(',')[2]
        let due = line.split(',')[3]
        
        // temporary
        let today = new Date()
        let next = new Date(start*1)
        if (freq == "m") {
            next.setMonth(today.getMonth() + 1)
        } else if (freq == "w") {
            next.setDate(today.getDate() + 7)
        } else if (freq == "b") {
            next.setDate(today.getDate() + 14)
        } else if (freq == "y") {
            next.setFullYear(today.getFullYear() + 1)
        }

        console.log(next)

        let bill = document.createElement("div")
        bill.classList.add("bill")
        bill.innerHTML = `${name} - $${price} - ${freq} - ${start}`
        billsDiv.appendChild(bill)
    }
    console.log(lines)
})()