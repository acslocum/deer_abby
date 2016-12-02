require_relative 'mad_lib'

class DeerAbby
  def initialize()
    @max_responders = 3
    @current_responder = 0
    @max_responses = 20
    @current_response = 0
    @voices = ["Agnes", "Princess", "Victoria"]
  end
  
  def respond(question)
    mad_lib = MadLib.new(next_response)
    puts next_responder
    response = mad_lib.fill(question)
    #puts response
    `say -v #{using_voice} "#{response}"`
    puts 'x'
  end
  
  def next_response
    @current_response = (@current_response+1) % @max_responses
  end
  
  def next_responder
    @current_responder = (@current_responder+1) % @max_responders
  end
  
  def using_voice
    @voices[@current_responder]
  end
end

deer = DeerAbby.new

while input = gets do
  deer.respond(input)
end