require 'mkmf'

dir_config('qrencode')

if have_library('qrencode', 'QRcode_encodeString')
  create_makefile('qrencode')
end
