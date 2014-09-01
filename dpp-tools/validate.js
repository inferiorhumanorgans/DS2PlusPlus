#!/usr/bin/env node

var tv4 = require ('tv4');
var fs = require('fs');
var sys = require('sys');
JSON.minify = JSON.minify || require("node-json-minify");

var jsonSchema = fs.readFileSync(process.argv[2]).toString();
jsonSchema = (JSON.minify(jsonSchema));

var jsonData = fs.readFileSync(process.argv[3]).toString();
jsonData = JSON.minify(jsonData);

var isValid = tv4.validate(jsonData, jsonSchema, true, true);

if (isValid != true) {
  console.log(isValid)
  console.log(tv4.error)  
}
