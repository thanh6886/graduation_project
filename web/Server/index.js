const express = require("express");
const cors = require("cors");
const mqtt = require("mqtt");
const mysql = require("mysql");
const bodyParser = require("body-parser");

const MQTTbroker = {
  host: "5becba34c368460ba7657c804a6e4eed.s2.eu.hivemq.cloud",
  username: "bé_thu",
  password: "Thanh2412",
  port: 8883,
  protocol: "mqtts",
};

const DataMysql = mysql.createConnection({
  host: "127.0.0.1",
  user: "root",
  password: "",
  database: "mqtt_esp32",
});

const app = express();
app.use(cors());
app.use(bodyParser.json());
const Client = mqtt.connect(MQTTbroker);

function handleConnectMySQL() {
  DataMysql.connect((err) => {
    if (err) {
      console.log("Lỗi khi kết nối đến cơ sở dữ liệu:", err);
      setTimeout(handleConnectMySQL, 1000);
    }
    console.log("Đã kết nối đến cơ sở dữ liệu!");
  });

  DataMysql.on("error", (err) => {
    console.log("Lỗi cơ sở dữ liệu", err);
    if (err.code === "PROTOCOL_CONNECTION_LOST") {
      handleConnectMySQL();
    } else {
      throw err;
    }
  });
}
handleConnectMySQL();

Client.on("connect", () => {
  Client.subscribe("DATA", (err) => {
    if (err) {
      console.log("Lỗi khi subscribe topic DATA", err);
    } else console.log("Đã subscribe topic DATA");
  });
  Client.subscribe("Status", (err) => {
    if (err) {
      console.log("Lỗi khi subscribe topic Status", err);
    } else console.log("Đã subscribe topic Status");
  });
});

Client.on("message", (topic, message) => {
  const msg = message.toString();
  console.log("[Topic arrived] " + topic);
  console.log("[Message arrived] " + msg);

  if (topic === "DATA") {
    DataMysql.query("SELECT Product_A, Product_B, Product_C FROM esp32 ORDER BY id DESC LIMIT 1", (err, rows) => {
      if (err) {
        console.error("Lỗi khi truy vấn cơ sở dữ liệu:", err.message);
        return;
      }

      const current = rows[0] || { Product_A: 0, Product_B: 0, Product_C: 0 };

      let updatedA = current.Product_A;
      let updatedB = current.Product_B;
      let updatedC = current.Product_C;

      if (msg === "A") updatedA++;
      if (msg === "B") updatedB++;
      if (msg === "C") updatedC++;

      const total = updatedA + updatedB + updatedC;

      
      INSERT_DATA({
        Product_A: updatedA,
        Product_B: updatedB,
        Product_C: updatedC,
        SUM: total
      });
    });
  }
  if(topic === "status"){
    const parts = msg.split(":");
    if (parts.length === 3 && parts[0] === "L") {
      const device = parts[1];
      const value_device = parseInt(parts[2], 10);
      switch(device){
        case "LA":
          break
        case "LB":
          break
        case "CAM":
          break;
        case "BT":
            break;
        case "AUTO":
              break;
        default:
          break;
      }
    }
  }
});

function INSERT_DATA(value) {
  const DataSQL = `
    INSERT INTO esp32 (datetime, Product_A, Product_B, Product_C, SUM) 
    VALUES (NOW(), ?, ?, ?, ?)
  `;


  DataMysql.query(DataSQL, [value.Product_A, value.Product_B, value.Product_C, value.SUM], (err, result) => {
    if (err) {
      console.error("Lỗi khi chèn dữ liệu:", err.message);
    } else {
      console.log("Chèn dữ liệu thành công từ sensor");
    }
  });
}

const server = app.listen(3000, () => {
  console.log(`Server đang chạy → PORT ${server.address().port}`);
});
