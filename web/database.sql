CREATE DATABASE mqtt_esp32;

CREATE TABLE esp32 (
    id  INT AUTO_INCREMENT PRIMARY KEY,
    datetime TIMESTAMP NOT NULL,
    Product_A   INT NOT NULL,
    Product_B   INT NOT NULL,
    Product_C   INT NOT NULL,
    SUM         INT NOT NULL,
);