require 'rubygems'
require 'engtagger'


class MadLib
  @nouns = ["fort", "cull", "city council", "zach", "fortress"]
  @verbs_infinitive = ["party", "explore"]
  @adjectives = ["weak", "green", "tawny", "sparkling"]
  
  
  def initialize(response_number)
    @response_number = response_number
  end
  
  def fill(question)
    raw_response = load_response
    word_hash = augment(parse(question))
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
      word_hash[pair[1]] = [] unless word_hash[pair[1]]
      word_hash[pair[1]] << pair[0] 
      end
    word_hash
  end
  
  def augment(word_hash)
    word_hash["NN"] = [@nouns.sample] unless(word_hash.include? "NN")
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