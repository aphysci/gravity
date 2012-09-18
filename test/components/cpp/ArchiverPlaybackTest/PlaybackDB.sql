-- MySQL dump 10.13  Distrib 5.5.27, for Win64 (x86)
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version	5.5.27

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `test`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `test` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `test`;

--
-- Table structure for table `test`
--

DROP TABLE IF EXISTS `PlaybackTest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `PlaybackTest` (
  `timestamp` bigint(20) DEFAULT NULL,
  `message` blob,
  `DataproductID` varchar(45) DEFAULT NULL,
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=174 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `PlaybackTest`
--

LOCK TABLES `PlaybackTest` WRITE;
/*!40000 ALTER TABLE `PlaybackTest` DISABLE KEYS */;
INSERT INTO `PlaybackTest` VALUES (1347650091146517,0x38,'DummyDataProduct',158),(1347650092159539,0x39,'DummyDataProduct',159),(1347650093174154,0x3130,'DummyDataProduct',160),(1347650094187821,0x3131,'DummyDataProduct',161),(1347650095202558,0x3132,'DummyDataProduct',162),(1347650096216020,0x3133,'DummyDataProduct',163),(1347650097230737,0x3134,'DummyDataProduct',164),(1347650098244436,0x3135,'DummyDataProduct',165),(1347650099257934,0x3136,'DummyDataProduct',166),(1347650100271939,0x3137,'DummyDataProduct',167),(1347650101286105,0x3138,'DummyDataProduct',168),(1347650102299973,0x3139,'DummyDataProduct',169),(1347650103314201,0x30,'DummyDataProduct',170),(1347650104327944,0x31,'DummyDataProduct',171),(1347650105341236,0x32,'DummyDataProduct',172),(1347650106355265,0x33,'DummyDataProduct',173);
/*!40000 ALTER TABLE `PlaybackTest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ArchiveTest`
--

DROP TABLE IF EXISTS `ArchiveTest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ArchiveTest` (
  `timestamp` bigint(20) DEFAULT NULL,
  `message` blob,
  `DataproductID` varchar(45) DEFAULT NULL,
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8;

/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-09-18 11:33:26
