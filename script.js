import { initializeApp } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyBzdxGZiIAs1opcuNzIYBIdCAulEyEssYQ",
  authDomain: "airqualitymonitor-df0db.firebaseapp.com",
  databaseURL: "https://airqualitymonitor-df0db-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "airqualitymonitor-df0db",
  storageBucket: "airqualitymonitor-df0db.appspot.com",
  messagingSenderId: "102056390066",
  appId: "1:102056390066:web:7f570d86855b3f619b88ab",
  measurementId: "G-74HVSPH2P9"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

document.addEventListener("DOMContentLoaded", () => {
  fetchLiveData();
  fetchHourlyData();
});

function fetchLiveData() {
  const liveRef = ref(db, "airquality/data");
  onValue(liveRef, snapshot => {
    const data = snapshot.val();
    console.log("Live Data:", data);
    if (data) updateLiveData(data);
    else console.warn("No live data found.");
  }, (error) => {
    console.error("Error fetching live data:", error);
  });
}

function fetchHourlyData() {
  const hourlyRef = ref(db, "hourly");
  onValue(hourlyRef, snapshot => {
    const data = snapshot.val();
    console.log("Hourly Data:", data);

    if (!data) {
      console.warn("No hourly data found.");
      return;
    }

    const hourlyArray = [];
    for (let time in data) {
      hourlyArray.push({ time, ...data[time] });
    }

    hourlyArray.sort((a, b) => a.time.localeCompare(b.time));
    updateHourlyTable(hourlyArray);
    updateAllCharts(hourlyArray);
  }, (error) => {
    console.error("Error fetching hourly data:", error);
  });
}

function updateLiveData(data) {
  document.getElementById("pm25-level").innerText = ${data.pm25} μg/m³;
  document.getElementById("pm10-level").innerText = ${data.pm10} μg/m³;
  document.getElementById("temperature").innerText = ${data.temperature} °C;
  document.getElementById("humidity").innerText = ${data.humidity} %;

  const badge = document.getElementById("air-quality-badge");
  badge.innerText = data.airQuality;
  badge.className = "badge";

  switch (data.airQuality) {
    case "Good":
      badge.classList.add("bg-success");
      break;
    case "Moderate":
      badge.classList.add("bg-warning");
      break;
    case "Unhealthy":
      badge.classList.add("bg-danger");
      break;
    default:
      badge.classList.add("bg-secondary");
  }
}

function updateHourlyTable(data) {
  const tableBody = document.getElementById("hourly-data-table");
  tableBody.innerHTML = "";
  data.forEach(entry => {
    const row = `
      <tr>
        <td>${entry.time}</td>
        <td>${entry.pm25}</td>
        <td>${entry.pm10}</td>
        <td>${entry.temperature}</td>
        <td>${entry.humidity}</td>
        <td>${entry.airQuality}</td>
      </tr>`;
    tableBody.innerHTML += row;
  });
}

let charts = {};

function updateAllCharts(data) {
  const times = data.map(d => d.time);

  const chartConfigs = [
    {
      id: "airQualityChart",
      label: ["PM2.5", "PM10"],
      datasets: [
        {
          label: "PM2.5",
          data: data.map(d => d.pm25),
          borderColor: "rgba(255, 99, 132, 1)",
          backgroundColor: "rgba(255, 99, 132, 0.2)"
        },
        {
          label: "PM10",
          data: data.map(d => d.pm10),
          borderColor: "rgba(54, 162, 235, 1)",
          backgroundColor: "rgba(54, 162, 235, 0.2)"
        }
      ]
    },
    {
      id: "pressureChart",
      label: "Pressure",
      datasets: [{
        label: "Pressure (hPa)",
        data: data.map(d => d.pressure),
        borderColor: "rgba(75, 192, 192, 1)",
        backgroundColor: "rgba(75, 192, 192, 0.2)"
      }]
    },
    {
      id: "mq7Chart",
      label: "MQ7",
      datasets: [{
        label: "MQ7 (ppm)",
        data: data.map(d => d.mq7),
        borderColor: "rgba(255, 159, 64, 1)",
        backgroundColor: "rgba(255, 159, 64, 0.2)"
      }]
    },
    {
      id: "mq135Chart",
      label: "MQ135",
      datasets: [{
        label: "MQ135 (ppm)",
        data: data.map(d => d.mq135),
        borderColor: "rgba(153, 102, 255, 1)",
        backgroundColor: "rgba(153, 102, 255, 0.2)"
      }]
    },
    {
      id: "aqiChart",
      label: "AQI",
      datasets: [{
        label: "AQI",
        data: data.map(d => d.aqi),
        borderColor: "rgba(255, 205, 86, 1)",
        backgroundColor: "rgba(255, 205, 86, 0.2)"
      }]
    }
  ];

  chartConfigs.forEach(config => {
    const ctx = document.getElementById(config.id)?.getContext("2d");
    if (!ctx) {
      console.warn(Canvas with id ${config.id} not found.);
      return;
    }

    if (charts[config.id]) charts[config.id].destroy();

    charts[config.id] = new Chart(ctx, {
      type: "line",
      data: {
        labels: times,
        datasets: config.datasets
      },
      options: {
        responsive: true,
        scales: {
          y: { beginAtZero: true },
          x: { title: { display: true, text: "Time" } }
        }
      }
    });
  });
}
