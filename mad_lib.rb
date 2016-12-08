require 'rubygems'
require 'engtagger'


class MadLib
  
  
  def initialize(response_number)
    @response_number = response_number
    @nouns = ["fort", "cull", "city council", "zach", "fortress"]
    @verbs_infinitive = ["party", "explore"]
    @adjectives = ["weak", "green", "tawny", "sparkling"]
  end
  
  def fill(question)
    raw_response = load_response
    word_hash = parse(question)
    #puts word_hash
    merge(raw_response, word_hash)
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
      key = normalize(pair[1])
      word_hash[key] = [] unless word_hash[key]
      word_hash[key] << pair[0] 
      end
    word_hash
  end
  
  def normalize(key)
    return "VB" if key.include? "VB"
    return "NN" if key.include? "NN"
    return "JJ" if key.include? "JJ"
    key
  end
  
  def augment(word_hash)
    word_hash["NN"] = [@nouns.sample] unless(word_hash.include? "NN")
    word_hash["NN"] << [@nouns.sample] if(word_hash["NN"].size < 2)
    word_hash["JJ"] = [@adjectives.sample] unless(word_hash.include? "JJ")
    word_hash["VB"] = [@verbs_infinitive.sample] unless(word_hash.include? "VB")
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