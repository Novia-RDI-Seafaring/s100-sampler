package com.example.secom;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.ComponentScan;

@SpringBootApplication
@ComponentScan(basePackages = "com.example.secom") // Ensure components are scanned
public class SecomNovia {
    public static void main(String[] args) {
        SpringApplication.run(SecomNovia.class, args); // Start the Spring Boot app
    }
}
