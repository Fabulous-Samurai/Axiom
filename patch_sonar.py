with open("sonar-project.properties", "r") as f:
    content = f.read()

content = content.replace('sonar.sources=src,include', 'sonar.sources=engine,scripts')

with open("sonar-project.properties", "w") as f:
    f.write(content)
