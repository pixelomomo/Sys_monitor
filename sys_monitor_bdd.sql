CREATE TABLE table_de_controle
(
	control_id INT PRIMARY KEY NOT NULL,
    ctrl_name VARCHAR(100),
    ctrl_desc LONGTEXT
);

CREATE TABLE table_de_parametre
(
	name VARCHAR(100),
    value VARCHAR(100),
    controlid INT REFERENCES table_de_controle(controle_id)
);

CREATE TABLE table_de_config
(
	config_id INT PRIMARY KEY NOT NULL,
    config_name VARCHAR(100),
    config_desc VARCHAR(1000)
);

CREATE TABLE table_de_config_ctrl
(
	configid INT REFERENCES table_de_config,
    controlid INT REFERENCES table_de_controle
);
