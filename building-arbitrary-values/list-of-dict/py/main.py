def hello_world(args):
    if len(args) != 2:
        print("invalid #(args)")

    for v in args:
        opaque = str(v["opaque"], encoding="ascii")
        # opaque = v["opaque"]
        print(f'value={v["value"]}, opaque="{opaque}"')


if __name__ == "__main__":
    _args = [
        {"value": 123, "opaque": b"hello"},
        {"value": 456, "opaque": b"world"},
    ]

    hello_world(_args)
