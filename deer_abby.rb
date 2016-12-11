require_relative 'mad_lib'
require 'rubygems'
require 'serialport'

class DeerAbby

  def initialize()
    @max_responders = 3
    @current_responder = 0
    @voices = ["Agnes", "Vicki", "Bruce"] #deer abby, lan antlers, dan staggage
    @on = 255
    @off = 0
    @delimiter = ':'
    @usb_port = "/dev/cu.usbmodem1451"
    @speed = 115200
    @serial = SerialPort.new(@usb_port,@speed,8,1,SerialPort::NONE)
  end
  
  def respond(question)
    mad_lib = MadLib.new
    next_responder
    response = mad_lib.fill(question)
    speak(response, @current_responder)
  end
  
  def speak(response, responder)
    @serial.puts(format_for_arduino(get_light_string(responder)))
    puts "\n\n#{response}"
    `say -v #{using_voice} "#{response}"`
  end
  
  def next_response
    @current_response = (@current_response+1) % @max_responses
  end
  
  def get_light_string(responder)
    result = case responder
    when 0
      [@delimiter, @on, @on, @on, @off, @off, @off, @off, @off, @off, @off, @off, @off] #1 - deer abby
    when 1
      [@delimiter, @off, @off, @off, @off, @off, @off, @on, @on, @on, @off, @off, @off] #2 - lan antlers
    else
      [@delimiter, @off, @off, @off, @off, @off, @off, @off, @off, @off, @on, @on, @on] #3 - dan staggage
    end
    result
  end
  
  def reset_light_string
    [@delimiter, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off]
  end 
  
  def format_for_arduino(fixnum_array)
    fixnum_array.collect{|x|x.chr}.join
  end
  
  def next_responder
    @current_responder = rand(@max_responders)
  end
  
  def using_voice
    @voices[@current_responder]
  end
end

deer = DeerAbby.new
prompt = "\n\n  About what do need advice, deer reader?\n\n"
puts prompt
putc ">"
while input = gets do
  if(input.length > 5)
    deer.respond(input)
  else
    deer.speak("   Please be a bit more verbose, deer.\n", 0)
  end
  puts prompt
  putc ">"
end