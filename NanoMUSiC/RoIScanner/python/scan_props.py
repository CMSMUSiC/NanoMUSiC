from pydantic import BaseModel


class ScanProps(BaseModel):
    ec_name: str
    distribution_type: str
    json_file_path: str
    output_directory: str
    rounds: int
    start_round: int = 0
