class GeneratedFile:
    def __init__(self, filename: str, type: str, content: str):
        self.filename = filename
        self.type = type
        self.content = content

class BaseCodegen:
    def __init__(self, name: str, data: dict, source_dir):
        raise NotImplementedError("You have to override this method")

    def exec(self) -> list[GeneratedFile]:
        raise NotImplementedError("You have to override this method")
