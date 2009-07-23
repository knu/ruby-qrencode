require 'qrencode.so'

class QRcode
  def empty_row(margin = 4)
    Array.new(margin + width + margin, false)
  end

  def each_row(margin = 4)
    block_given? or return to_enum(__method__, margin)

    if 0 < margin
      margin.times {
        yield empty_row(margin)
      }

      width.times { |y|
        row = empty_row(margin)
        row[margin...-margin] = (0...width).map { |x| self[x, y] }
        yield row
      }

      margin.times {
        yield empty_row(margin)
      }
    else
      width.times { |y|
        row = (0...width).map { |x| self[x, y] }
        yield row
      }
    end

    self
  end

  def each(margin = 4)
    block_given? or return to_enum(__method__, margin)

    each_row(margin).with_index { |row, y|
      row.each.with_index { |filled, x|
        yield [x, y, filled]
      }
    }

    self
  end

  include Enumerable

  REVERSE_TEXT = "\033[7m%s\033[m"

  def draw_text(options = {})
    margin = options[:margin] || 4

    text = ''
    each_row(margin) { |row|
      row.each { |filled|
        if filled
          text << '  '
        else
          text << REVERSE_TEXT % '  '
        end
      }
      text << "\n"
    }
    text
  end
end
