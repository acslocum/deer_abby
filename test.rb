require 'rubygems'
require 'engtagger'

# Create a parser object
tgr = EngTagger.new

# Sample text
#text = "Alice chased the big fat cat."
text = "Deer Abby, I really want to propose to my girlfriend, but I'm worried because I'm a young buck and her husband is an avid hunter. What should I do? Confused, Thumper."

puts text

# Add part-of-speech tags to text
tagged = tgr.add_tags(text)

#=> "<nnp>Alice</nnp> <vbd>chased</vbd> <det>the</det> <jj>big</jj> <jj>fat</jj><nn>cat</nn> <pp>.</pp>"

# Get a list of all nouns and noun phrases with occurrence counts
word_list = tgr.get_words(text)

#=> {"Alice"=>1, "cat"=>1, "fat cat"=>1, "big fat cat"=>1}

# Get a readable version of the tagged text
readable = tgr.get_readable(text)


nouns = tgr.get_nouns(tagged)

puts "\nnouns: #{nouns}"

proper = tgr.get_proper_nouns(tagged)

puts "proper nouns: #{proper}"

adj = tgr.get_adjectives(tagged)

puts "adjectives: #{adj}"

nps = tgr.get_noun_phrases(tagged)

puts "noun phrases: #{nps}"


puts "\n\ntagged text: #{readable}"

