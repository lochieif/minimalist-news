
// Fetch data from the server
fetch('localhost:8080/daily_news')
    .then(response => response.text())
    .then(data => {
        // Assuming the data is HTML-formatted links
        document.getElementById('news').innerHTML = data;
    })
    .catch(error => {
        console.error('Error fetching news:', error);
    });
