from pydantic import BaseModel


class ScanProps(BaseModel):
    ec_name: str
    json_file_path: str
    output_directory: str
    rounds: int
    start_round: int = 0
