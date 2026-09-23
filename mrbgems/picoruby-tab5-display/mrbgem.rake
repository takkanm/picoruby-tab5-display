MRuby::Gem::Specification.new('picoruby-tab5-display') do |spec|
  spec.license = 'MIT'
  spec.author  = 'Mitsutaka Mimura'
  spec.summary = 'Display class for M5Stack Tab5'

  # Make it loadable as require "display". The default would be derived from
  # the gem name as "tab5-display", which is awkward with the hyphen.
  spec.require_name = 'display'
end
