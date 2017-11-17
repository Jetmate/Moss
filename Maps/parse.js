const fs = require('fs')
const parseMagicaVoxel = require('parse-magica-voxel')
const path = require('path')

fs.readdir(path.resolve(__dirname, 'maps'), (err, items) => {
  if (err) throw err

  for (let item of items) {
    fs.readFile(path.resolve(__dirname, 'maps', item), (err, Buffer) => {
      if (err) throw err
      const result = parseMagicaVoxel(Buffer)

      console.log(result)
      const colors = result.XYZI.reduce((acc, val) => {
        console.log(val.c)
        if (!acc.includes(val.c)) acc.push(val.c)
        return acc
      }, [])

      const final = {
        blocks: result.XYZI.map(block => ({
          x: block.x,
          y: block.y,
          z: block.z,
          c: colors.indexOf(block.c),
        })),
        colors: colors.map(index => result.RGBA[index]),
      }

      fs.writeFileSync(
        path.resolve(__dirname, '../native-Build/bin/Data/Maps', 'file' + '.json'),
        JSON.stringify(final), null, 2
      )
    })
  }
})
