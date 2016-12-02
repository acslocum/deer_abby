require 'rubygems'
require 'engtagger'


class MadLib
  def initialize(response_number)
    @response_number = response_number
  end
  
  def say(question, voice)
    raw_response = load_response
    word_hash = parse(question)
    #puts word_hash
    response = merge(raw_response, word_hash)
    #puts "#{voice}: #{response}"
    value = `say -v #{voice} "#{response}"`
  end
  
  def load_response
    responses = Dir.entries("responses")
    file_number = (@response_number % (responses.size-2))+2
    File.open("responses/#{responses[file_number]}").gets
  end
  
  def parse(question)
    tgr = EngTagger.new
    tagged = tgr.get_readable(question)
    extracted = tagged.scan(/(\w+)\/(\w+)/)
    word_hash = {}
    extracted.each do |pair| 
      word_hash[pair[1]] = [] unless word_hash[pair[1]]
      word_hash[pair[1]] << pair[0] 
      end
    word_hash
  end
  
  def merge(raw_response, word_hash)
    tokens = tokenize(raw_response)
    response = raw_response.gsub(/\{(.+?)\}/) do |match|
      word_hash[$1].sample
    end
  end
  
  def tokenize(string)
    string.scan(/\{(.+?)\}/).flatten
  end
end