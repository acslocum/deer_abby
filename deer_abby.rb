require_relative 'mad_lib'
require 'rubygems'
require 'serialport'

class DeerAbby
  @on = 255
  @off = 1
  @delimiter = ':'
  @usb_port = '/dev/tty.usbmodem1411'
  @speed = 115200

  def initialize()
    @max_responders = 3
    @current_responder = 0
    @max_responses = 20
    @current_response = 0
    @voices = ["Agnes", "Princess", "Deranged"]
  end
  
  def respond(question)
    mad_lib = MadLib.new(next_response)
    next_responder
    serial = SerialPort.new(@usb_port,@speed,8,1,SerialPort::NONE)
    sleep(2)
    serial.puts(get_light_string)
    puts serial.readline
    response = mad_lib.fill(question)
    `say -v #{using_voice} "#{response}"`
    serial.puts(reset_light_string)
  end
  
  def next_response
    @current_response = (@current_response+1) % @max_responses
  end
  
  def get_light_string
    result = case @current_responder
    when 0
      [@delimiter, @on, @on, @on, @off, @off, @off, @off, @off, @off, @off, @off, @off] #1
    when 1
      [@delimiter, @off, @off, @off, @on, @on, @on, @off, @off, @off, @off, @off, @off] #2
    else
      [@delimiter, @off, @off, @off, @off, @off, @off, @on, @on, @on, @off, @off, @off] #3
    end
    result.collect { |x| x.chr }.join
  end
  
  def reset_light_string
    [@delimiter, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off, @off].collect { |x| x.chr }.join
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