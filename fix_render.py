import os, glob

files = [
    'src/rendering/LandingScreen.h',
    'src/rendering/LandingPanel.h',
    'src/rendering/InfoPanel.h',
    'src/rendering/MarketPanel.h',
    'src/rendering/OutfitterPanel.h',
    'src/rendering/ShipyardPanel.h',
    'src/rendering/RenderSystem.h',
]

for f in files:
    with open(f, 'r') as file:
        content = file.read()
    content = content.replace('sf::RenderWindow &window', 'sf::RenderTarget &target')
    
    # Also add #include <SFML/Graphics/RenderTarget.hpp> if there is RenderWindow
    content = content.replace('<SFML/Graphics/RenderWindow.hpp>', '<SFML/Graphics/RenderTarget.hpp>')
    
    with open(f, 'w') as file:
        file.write(content)

