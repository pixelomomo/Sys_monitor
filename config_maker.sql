CREATE TABLE `table_des_config` (
	`config_id` INTEGER NOT NULL,
	`name` VARCHAR(100) DEFAULT NULL,
	`desc` VARCHAR(1000) DEFAULT NULL,
	PRIMARY KEY(`config_id`)
);

CREATE TABLE `table_des_config_ctrl` (
	`configid` INTEGER NOT NULL,
	`ctrlid` INTEGER NOT NULL,
	PRIMARY KEY (`configid`, `ctrlid`),  -- Clé primaire composée
	UNIQUE (`ctrlid`)  -- Ajouter un index unique sur ctrlid
);

CREATE TABLE `table_des_controles` (
	`control_id` INTEGER NOT NULL,
	`name` VARCHAR(100) DEFAULT NULL,
	`description` VARCHAR(1000) DEFAULT NULL,
	PRIMARY KEY(`control_id`)
);

CREATE TABLE `table_des_parametres` (
	`param_id` INTEGER NOT NULL,
	`name` VARCHAR(100) NOT NULL,
	`value` VARCHAR(45) DEFAULT NULL,
	`value_2` VARCHAR(45) DEFAULT NULL,
	`control_id` INTEGER NOT NULL,  -- Renommé sans tiret
	PRIMARY KEY(`param_id`),
	FOREIGN KEY(`control_id`) REFERENCES `table_des_controles`(`control_id`)
		ON UPDATE NO ACTION ON DELETE NO ACTION
);

-- Ajout des clés étrangères correctes
ALTER TABLE `table_des_controles`
ADD FOREIGN KEY(`control_id`) REFERENCES `table_des_config_ctrl`(`ctrlid`)
ON UPDATE NO ACTION ON DELETE NO ACTION;

ALTER TABLE `table_des_config`
ADD FOREIGN KEY(`config_id`) REFERENCES `table_des_config_ctrl`(`configid`)
ON UPDATE NO ACTION ON DELETE NO ACTION;
