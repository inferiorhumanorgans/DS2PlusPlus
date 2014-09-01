#!/usr/bin/env ruby

require 'jschema'

require 'json'

schema = JSchema.build(JSON.parse(File.read(ARGV[0])))

schema.validate(JSON.parse(File.read(ARGV[1]))).each do |x|
  puts x
  puts
end
