require 'nokogiri'

# {items={{id="foo"}, {id="bar"}}}

def parse_node(node, level)
  id = "#{'  ' * level}#{node.attribute('id')}"
  url = ''
  matrix = ''

  node.children.each do |node_child|
    name = node_child.name
    if name == 'instance_geometry'
      url = "#{'  ' * level}#{node_child.attribute('url')}"
    elsif name == 'matrix'
      matrix = "#{'  ' * level}#{node_child.content}"
    elsif name == 'node'
      parse_node(node_child, level + 1)
    end
  end

  #puts "id: #{id}"
  #puts "url: #{url.downcase.gsub(/-mesh/, '.ply').gsub(/#/, '')}"
  #puts "matrix: #{matrix}"
  puts "{id=#{id}, url=\"#{url.downcase.gsub(/-mesh/, '.ply').gsub(/#/, '')}\", matrix={#{matrix.split(/\s+/).join(",")}}},"
end

file = ARGV[0]
File.open(file) do |file|
  doc = Nokogiri::XML(file.read)
  doc.remove_namespaces!
  #puts doc.xpath('//node').count
  puts "level = {items={"
  doc.xpath('//node').each do |node|
    parse_node(node, 0)
  end
  puts "}}"
end
