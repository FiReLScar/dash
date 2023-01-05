(async () => {
    let bank = await fetch('/api/data')
    bank = await bank.json()
    console.log(bank)
    document.getElementById('total-bal').innerHTML = "$" + (parseFloat(bank.Balance.Accounts[1].bal.substring(1)) + parseFloat(bank.Balance.Accounts[0].bal.substring(1))).toLocaleString()
    document.getElementById('checkings-bal').innerHTML = bank.Balance.Accounts[0].bal.toLocaleString()
    document.getElementById('savings-bal').innerHTML = bank.Balance.Accounts[1].bal.toLocaleString()

    let income = 0, expenses = 0

    for (let i = 0; i < bank.History.length; i++) {
        for (let j = 0; j < bank.History[i].length; j++) {
            let item = bank.History[i][j]
            if (item["money"][0] == '-') expenses += parseFloat(item["money"].substring(2))
            else income += parseFloat(item["money"].substring(1))
        }
    }

    document.getElementById('income').innerHTML = "$" + income.toLocaleString()
    document.getElementById('expenses').innerHTML = "$" + expenses.toLocaleString()

    let plus = '<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" style="color: #70f570" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>'
    let minus = '<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" style="color: #d44a4a" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M19.5 12h-15" /></svg>'
    let pending = '<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M2.25 15a4.5 4.5 0 004.5 4.5H18a3.75 3.75 0 001.332-7.257 3 3 0 00-3.758-3.848 5.25 5.25 0 00-10.233 2.33A4.502 4.502 0 002.25 15z" /></svg>'
    let check = '<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M9 12.75L11.25 15 15 9.75M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>'

    let t = document.getElementById('transactions')

    for (let i=0; i<3; i++) {
        let item = bank.History[0][i]
        let div = document.createElement('div')
        let div2 = document.createElement('div')
        div2.innerHTML = (item["money"][0] == '-' ? minus : plus)
        let div3 = document.createElement('div')
        let label = document.createElement('span')
        label.innerHTML = item["desc"]
        label.className = "label"
        let date = document.createElement('span')
        date.innerHTML = item["date"]
        date.className = "date"
        div3.appendChild(label)
        div3.appendChild(date)
        div2.appendChild(div3)
        div.appendChild(div2)

        let div4 = document.createElement('div')
        let amount = document.createElement('span')
        amount.innerHTML = item["money"]
        amount.className = "amount"
        div4.appendChild(amount)
        div4.innerHTML += (item["pending"] == true ? pending : check)
        div.appendChild(div4)

        t.appendChild(div)
    }
})()