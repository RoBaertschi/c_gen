import
console.log(process.argv)
const actualArgs = process.argv.slice(2)
const input = actualArgs[0]
const output = actualArgs[1]

if (!input) {
	console.error("missing first argument for input")
	process.exit(1)
}

if (!output) {
	console.error("missing first argument for output")
	process.exit(1)
}

const content = reafile