const emoji = require('./preset_emoji'),
  PRESET_BANK_SIZE = 8

module.exports = function (options, defines) {
  const presetDesc = `It can be mapped in only EC preset map layers(${
    defines.DYNAMIC_KEYMAP_LAYER_COUNT - defines.EC_NUM_PRESET_MAPS
  } - ${defines.DYNAMIC_KEYMAP_LAYER_COUNT - 1}).`
  return [
    {
      name: 'EC\nCalD',
      title: 'Show calibration data as keystrokes',
      shortName: 'EC.C'
    },
    {
      name: 'EC\nPSet',
      title: 'Show presets as keystrokes',
      shortName: 'EC.P'
    },
    // preset map selector keys
    ...Array(defines.EC_NUM_PRESET_MAPS)
      .fill(0)
      .map((_, i) => ({
        name: `EC\n Map ${i}`,
        title: `Select EC preset map ${i}.`,
        shortName: `ECM(${i})`
      })),
    // not keycode, preset
    ...Array(defines.EC_NUM_PRESETS)
      .fill(0)
      .map((_, presetIndex) => {
        const bank = (presetIndex / PRESET_BANK_SIZE) | 0,
          index = presetIndex % PRESET_BANK_SIZE,
          icon = emoji[bank % emoji.length][index % emoji[bank].length]
        return {
          name: `EC${bank}${index}\n${icon}`,
          title: `EC Preset Bank${bank} - ${index}. ${presetDesc}`,
          shortName: `${bank}${index} ${icon}`
        }
      }),
    ...(!defines.EC_DEBUG
      ? []
      : [
          {
            name: 'EC\nD.dt',
            title: 'Show debug data as keystrokes',
            shortName: 'EC.Dd'
          },
          {
            name: 'EC\nD.fq',
            title: 'Show matrix scan frequency as keystrokes',
            shortName: 'EC.Df'
          }
        ])
  ]
}
