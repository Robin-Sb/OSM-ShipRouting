var map = L.map('map').setView([-20, 140], 3);

let active = "none";
let coordsStart;
let coordsEnd;

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

function updateStart(newValue) {
    coordsStart = newValue;
    let newLat = coordsStart.lat || "None";
    let newLng = coordsStart.lng || "None";
    document.getElementById("start_display").innerHTML = "Latitude: " + newLat + ", Longitude: " + newLng;
}

function updateEnd(newValue) {
    coordsEnd = newValue;
    let newLat = coordsEnd.lat || "None";
    let newLng = coordsEnd.lng || "None";
    document.getElementById("end_display").innerHTML = "Latitude: " + newLat + ", Longitude: " + newLng;
}


async function startRequest() {
    //event.preventDefault();
    if (!coordsStart || !coordsEnd)
        return;
    // let lat1Val = document.getElementById("lat1").value;
    // let lon1Val = document.getElementById("lon1").value;
    // let lat2Val = document.getElementById("lat2").value;
    // let lon2Val = document.getElementById("lon2").value;
    let url = "http://localhost:8080?" + new URLSearchParams({
        lat1: coordsStart.lat,
        lon1: coordsStart.lng,
        lat2: coordsEnd.lat,
        lon2: coordsEnd.lng
    });
    const response = await fetch(url, {
        method: "GET"
    });
    // const response = await fetch("http://localhost:8080?lat1=2.1718&lon1=136.6390&lat2=-52.8316&lon2=164.0234", {
    //     method: "GET"
    // });
    let jsonData = await response.json();
    let distance = jsonData.distance;
    document.getElementById("dist_display").innerHTML = "Distance: " + distance + "m";
    delete jsonData.distance;
    console.log(distance);
    
    var myStyle = {
        "color": "#ff7800",
        "weight": 5,
        "opacity": 0.65
    };
    
    L.geoJSON(jsonData, { style: myStyle }).addTo(map)
    map.setView(new L.LatLng(coordsStart.lat, coordsEnd.lng), 3);
}


map.on("click", (event) => {
    if (active == "start")
        updateStart(event.latlng);
    else if (active == "end")
        updateEnd(coordsEnd = event.latlng);
    console.log(coordsStart);
    console.log(coordsEnd);
})

// let form = document.getElementById("locations");
// form.addEventListener("submit", startRequest)

document.getElementById("set_start").addEventListener("click", (event) => {
    active = "start";
});

document.getElementById("set_end").addEventListener("click", (event) => {
    active = "end";
});

document.getElementById("route").addEventListener("click", (event) => {
    startRequest();
    active = "none";
});


