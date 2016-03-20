TLVLength = (function() {
	function TLVLength() {
		if (arguments.length) {
			if (jQuery.isArray(arguments[0]))
				this.binary = arguments[0];
			else
				this.value = arguments[0];
		}
	}

	function setBinary(binary) {
		if (!binary)
			return;

		if ((binary[0] & TLVLength.LONG_FORM) !== TLVLength.LONG_FORM) {
			this.value = binary[0];
		} else {
			var length = 1 + (binary[0] & TLVLength.LONG_FORM_LENGTH_MASK);
			this.value = 0;

			for (var i = 1; i < length; i++)
				this.value = this.value * 256 + binary[i];
		}
	}

	function getBinary() {
		var result = [];

		if (this.value < TLVLength.LONG_FORM) {
			result.push(this.value);
		} else {
			var value = this.value;
			var len = 0;

			while (value > 0) {
				result[len++] = value % 256;
				value = Math.floor(value / 256);
			}

			result[len] = TLVLength.LONG_FORM | len;

			result.reverse();
		}

		return result;
	}

	TLVLength.LONG_FORM = 0x80;
	TLVLength.LONG_FORM_LENGTH_MASK = 0x7F;

	TLVLength.prototype.constructor = TLVLength;

	Object.defineProperties(TLVLength.prototype, {
		binary: {
			set: setBinary,
			get: getBinary,
			enumerable: true,
			configurable: true,
		}
	});

	return TLVLength;
})();

TLVTag = (function() {
	function TLVTag(der) {
		this[0] = der[0];
		this.length = 1;

		if ((der[0] & TLVTag.NUMBER_MASK) === TLVTag.NUMBER_MASK) {
			do {
				this[this.length] = der[this.length];
			} while (der[this.length++] & TLVTag.CONTINUE == TLVTag.CONTINUE);
		}
	}

	function getBinary() {
		var binary = [];

		for (var i = 0; i < this.length; i++)
			binary.push(this[i]);

		return binary;
	}

	TLVTag.NUMBER_MASK = 0x1F;
	TLVTag.CONSTRUCTED = 0x20;
	TLVTag.CONTINUE = 0x80;

	TLVTag.prototype.constructor = TLVTag;

	TLVTag.prototype.isConstructed = function() {
		return !!(this[0] & TLVTag.CONSTRUCTED);
	}

	Object.defineProperties(TLVTag.prototype, {
		binary: {
			get: getBinary,
			enumerable: true,
			configurable: true
		}
	});

	return TLVTag;
})();

TLVNode = (function() {
	function TLVNode(binary) {
		this.binary = binary;
	}

	function toHex(b) {
		var hex= [ '0', '1', '2', '3', '4', '5', '6', '7',
			   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' ];

		return hex[Math.floor(b / 16)] + hex[b % 16];
	}

	function binToHex(bin) {
		return bin.reduce(function(prev, cur, idx, arr) { return prev + toHex(cur); }, "");
	}

	function setBinary(binaryDer) {
		var len, valueSlice, idxValueStart, idxValueEnd;

		if (binaryDer === undefined)
			return undefined;

		this.tag = new TLVTag(binaryDer);
		this.label = binToHex(this.tag.binary);

		len = new TLVLength(binaryDer.slice(this.tag.length));

		idxValueStart = this.tag.length + len.binary.length;
		idxValueEnd = idxValueStart + len.value;
		valueSlice = binaryDer.slice(idxValueStart, idxValueEnd);

		if (this.tag.isConstructed()) {
			delete this.value;
			this.children = [];

			while (idxValueStart < idxValueEnd) {
				var node;

				valueSlice = binaryDer.slice(idxValueStart, idxValueEnd);
				node = new TLVNode(valueSlice);
				this.children.push(node);
				idxValueStart += node.length;
			}
		} else {
			delete this.children;
			this.value = valueSlice;
			this.label += " " + binToHex(this.value);
		}
	}

	function getBinary() {
		var result = this.tag.binary;

		if (this.tag.isConstructed()) {
			var childrenResult = [];

			for (var i = 0; i < this.children.length; i++)
				childrenResult = childrenResult.concat(this.children[i].binary);

			result = result.concat(new TLVLength(childrenResult.length).binary);
			result = result.concat(childrenResult);
		} else {
			result = result.concat(new TLVLength(this.value.length).binary);
			result = result.concat(this.value);
		}

		return result;
	}

	function getLength() {
		var valueLength = 0;

		if (this.tag.isConstructed()) {
			for (var i = 0; i < this.children.length; i++)
				valueLength += this.children[i].length;
		} else {
			valueLength = this.value.length;
		}

		return this.tag.length + new TLVLength(valueLength).binary.length + valueLength;
	}

	TLVNode.prototype.constructor = TLVNode;

	Object.defineProperties(TLVNode.prototype, {
		binary: {
			set: setBinary,
			get: getBinary,
			enumerable: true,
			configurable: true
		},
		length: {
			get: getLength,
			enumerable: true,
			configurable: true
		}
	});

	return TLVNode;
})();
