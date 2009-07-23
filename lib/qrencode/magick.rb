require 'qrencode'
begin
  require 'RMagick'
rescue LoadError
  require 'rubygems'
  require 'RMagick'
end

class QRcode
  def draw_magick(options = {})
    margin           = options[:margin] || 4
    cell_size        = options[:cell_size] || 2
    foreground_color = options[:foreground_color] || '#000000'
    background_color = options[:background_color] || '#ffffff'

    size = (margin + width + margin) * cell_size

    image = Magick::Image.new(size, size) {
      self.background_color = background_color
    }
    gc = Magick::Draw.new
    gc.fill(foreground_color)

    each(margin) { |x, y, filled|
      if filled
        left = x * cell_size
        top  = y * cell_size
        gc.rectangle(left, top, left + cell_size - 1, top + cell_size - 1)
      end
    }

    gc.draw(image)
    image
  end
end
