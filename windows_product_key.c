#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/platform_device.h>

#include <linux/acpi.h>

static ssize_t winkey_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	int i;
	struct acpi_table_header *table;
	struct acpi_table_msdm *msdm;
	u8 *body;
	u32 body_len;
	u8 *product_code;
	u32 product_code_len;
	acpi_status status;

	status = acpi_get_table(ACPI_SIG_MSDM, 0, &table);

	if (ACPI_FAILURE(status)) {
		const char *msg = acpi_format_exception(status);

		pr_err("Failed to get MSDM table, %s\n", msg);
		return -ENODEV;
	}

	msdm = (struct acpi_table_msdm *)table;

	body = (u8 *) (table) + sizeof(struct acpi_table_header);
	body_len = msdm->header.length - sizeof(struct acpi_table_header);

	/* scan for characters that look like a product key beyond the header */
	for (i = 0; i < body_len; i++) {
		if (isalnum(body[i]))
			break;
	}

	if (i == body_len)
		return -ENODEV;

	product_code = body + i;
	product_code_len = body_len - i;

	return snprintf(buf, PAGE_SIZE, "%.*s\n",
			product_code_len, product_code);

}

static DEVICE_ATTR(key, S_IRUSR, winkey_show, NULL);

static struct platform_device *winkey_dev;

static int __init
windows_product_key_init(void)
{
	int ret;

	winkey_dev = platform_device_register_simple(
			"windows_product_key",
			-1, NULL, 0);
	if (IS_ERR(winkey_dev)) {
		ret = PTR_ERR(winkey_dev);
		winkey_dev = NULL;
		pr_err("unable to register windows_product_key platform "
				"device\n");
		return ret;
	}

	ret = device_create_file(&winkey_dev->dev, &dev_attr_key);
	if (ret) {
		pr_err("unable to create sysfs windows_product_key device "
				"attributes\n");
		return ret;
	}

	return 0;
}

static void __exit
windows_product_key_exit(void)
{
	if (winkey_dev)
		platform_device_unregister(winkey_dev);
}

module_init(windows_product_key_init);
module_exit(windows_product_key_exit);

MODULE_LICENSE("GPL");
