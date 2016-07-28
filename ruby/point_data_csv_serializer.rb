require 'csv'

# Serializes multiple point data series to CSV
#
# Each series may be sampled at different times. For instance, one series might
# be sampled every 5 minutes while another is every 10 minutes. For the CSV
# export, we step through each series and create a row that's filled in for any
# point that has data for that timestamp.
#
# For example, you might get exports like:
#
#     time,Point1,Point2
#     0:00,10,
#     0:05,20,50
#     0:10,30,
#     0:15,40,55
#
# To achieve this, we create an `Enumerator` for each series. The `Enumerator`
# keeps track of where the next item to be written is. Keep in mind that we may
# step through each series at a different rate.
class PointDataCSVSerializer
  def initialize(point_data_series)
    # { PointDataSeries => Enumerator, ... }
    @series_data_enumerators = Hash[
      point_data_series.map { |series| [series, series.point_datas.to_enum] }
    ]
  end

  def to_csv
    CSV.generate(headers: ['time'] + point_names, write_headers: true) do |csv|
      rows.each do |row|
        timestamp = row.values.first.timestamp
        csv << Hash[
          row.map { |series, point_data| [series.point.name, formatted_value(point_data)] }
        ].merge!('time' => timestamp)
      end
    end
  end

  private
  def rows
    return enum_for(:rows) unless block_given?

    loop do
      # Find the minimum step of time and select only those series which have
      # values at that timestamp
      next_chunk = @series_data_enumerators.map { |series, enum| [series, enum, peek_timestamp(series)] }.
        reject  { |_series, _enum, timestamp| timestamp.nil? }.
        sort_by { |_series, _enum, timestamp| timestamp }.
        chunk   { |_series, _enum, timestamp| timestamp }.
        first

      if next_chunk
        yield Hash[next_chunk.last.map { |series, enum, _timestamp| [series, enum.next] }]
      else
        break
      end
    end
  end

  def formatted_value(point_data)
    case point_data.value
    when nil then nil
    else          point_data.value.round(2)
    end
  end

  def peek_timestamp(series)
    if enumerator = @series_data_enumerators[series]
      enumerator.peek.timestamp
    else
      nil
    end
  rescue StopIteration
    @series_data_enumerators.delete(series)
    nil
  end

  def point_names
    @series_data_enumerators.map { |series, _enum| series.point.name }
  end
end
