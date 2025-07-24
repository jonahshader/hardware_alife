#!/usr/bin/env python3

import numpy as np
from PIL import Image, ImageDraw, ImageFont
import os


def create_font_atlas():
  # Font configuration
  font_size = 12
  char_width = 8
  char_height = 12
  padding = 2  # 2px padding around each character
  cell_width = char_width + padding * 2   # 12px total
  cell_height = char_height + padding * 2  # 16px total
  chars_per_row = 16
  atlas_size = 256

  # Create white atlas
  atlas = np.ones((atlas_size, atlas_size, 4), dtype=np.uint8) * 255

  # Characters to include (printable ASCII 32-126)
  chars = [chr(i) for i in range(32, 127)]

  # Try to use a system font, fallback to default
  font = None
  font_paths = [
      "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
      "/System/Library/Fonts/Monaco.ttf",
      "/Windows/Fonts/consola.ttf"
  ]

  for font_path in font_paths:
    if os.path.exists(font_path):
      try:
        font = ImageFont.truetype(font_path, font_size)
        print(f"Using font: {font_path}")
        break
      except:
        continue

  if font is None:
    try:
      font = ImageFont.load_default()
      print("Using default font")
    except:
      print("Could not load any font, creating simple pattern atlas")
      # Create simple pattern atlas without text
      for i, char in enumerate(chars):
        row = i // chars_per_row
        col = i % chars_per_row

        y_start = row * char_height
        x_start = col * char_width

        # Create simple pattern for each character
        for y in range(char_height):
          for x in range(char_width):
            if (x + y) % 3 == ord(char) % 3:
              atlas[y_start + y, x_start + x] = [255, 255, 255, 255]
            else:
              atlas[y_start + y, x_start + x] = [0, 0, 0, 0]

      # Save atlas
      img = Image.fromarray(atlas, 'RGBA')
      img.save(
          '/home/jonah-shader/repos/conf_demo/software/ps_display/resources/font_atlas.png')
      print("Created pattern-based font atlas")
      return

  # Create image for drawing text
  img = Image.new('RGBA', (atlas_size, atlas_size), (0, 0, 0, 0))
  draw = ImageDraw.Draw(img)

  # Draw each character with padding
  for i, char in enumerate(chars):
    row = i // chars_per_row
    col = i % chars_per_row

    # Position with padding (character drawn at center of padded cell)
    cell_x = col * cell_width
    cell_y = row * cell_height
    char_x = cell_x + padding
    char_y = cell_y + padding

    try:
      draw.text((char_x, char_y), char, font=font, fill=(255, 255, 255, 255))
    except:
      # Fallback for characters that can't be drawn
      draw.rectangle([char_x, char_y, char_x + char_width - 1, char_y + char_height - 1],
                     outline=(128, 128, 128, 255))

  # Save the atlas
  img.save(
      '/home/jonah-shader/repos/conf_demo/software/ps_display/resources/font_atlas.png')
  print(f"Font atlas created: font_atlas.png ({atlas_size}x{atlas_size})")


if __name__ == "__main__":
  create_font_atlas()
